#include <stdio.h>
#include "em_device.h"
#include "em_assert.h"
#include "em_gpio.h"

#include "em_leuart.h"

#include "ringbuffer.h"

#include "console_leuart.h"

/*
 * The LEUART is a slow device with no FIFO. We have to supply
 * a layer of buffering on top.   Thats probably for the best,
 * as it'll make flow control easier.
 */

// This driver supports one primary UART(0) and one
// Virtual uart (1) so that other tasks can
// have access to UART output while in the background.

#define RX_FIFOSIZE 16 // Doesn't need to be very big.
#define TX_FIFOSIZE 32

static uint8_t rb_storage_rx[2][RX_FIFOSIZE];
static uint8_t rb_storage_tx[2][TX_FIFOSIZE];

RINGBUF rb_rx[2];
RINGBUF rb_tx[2];

typedef struct {
    unsigned long *tcb;
    unsigned blocked_count_tx;
    unsigned blocked_count_rx;
    unsigned xoff_count;
    RINGBUF *rb_rx;
    RINGBUF *rb_tx;
    bool blocked_tx;
    bool blocked_rx;
    uint8_t pended_fc_char; // Send at the next opportunity.
} sIOBlockingData;

// Support multiple descriptors.
sIOBlockingData connection_state[2];

void forth_thread_stop(sIOBlockingData *s) {
    s->tcb[2] &= ~1; // Clear it using the unsafe technique.
}

void forth_thread_restart(sIOBlockingData *s) {
    s->tcb[2] |= 1;
}

void console_leuart_spin() {
    bool busy;
    do {
        busy = (LEUART0->STATUS & LEUART_STATUS_TXBL) == 0 ;
    } while ( busy );
}


/* LEUART1 initialization data */
const LEUART_Init_TypeDef leuart0Init = {
    .enable   = leuartEnable,       /* Activate data reception on LEUn_TX pin. */
    .refFreq  = 0,                    /* Inherit the clock frequenzy from the LEUART clock source */
    .baudrate = 9600,                 /* Baudrate = 9600 bps */
    .databits = leuartDatabits8,      /* Each LEUART frame containes 8 databits */
    .parity   = leuartNoParity,       /* No parity bits in use */
    .stopbits = leuartStopbits1,      /* Setting the number of stop bits in a frame to 2 bitperiods */
};

// ------------- Statistics ------------------
int count_leuart_irqs = 0;

void initLeuart(void) {
    /* Reseting and initializing LEUART0 */
    LEUART_Reset(LEUART0);
    LEUART_Init(LEUART0, &leuart0Init);

    /* Route LEUART0 TX pin to DMA location 0 */
    LEUART0->ROUTE = LEUART_ROUTE_TXPEN | LEUART_ROUTE_RXPEN | LEUART_ROUTE_LOCATION_LOC0;

    /* Enable TX Completion and RX Data */
    LEUART_IntEnable(LEUART0, LEUART_IEN_RXDATAV | LEUART_IEN_TXC);

    /* Enable GPIO for LEUART0. TX is on C6 */
    GPIO_PinModeSet(gpioPortD,                /* GPIO port */
                    4,                        /* GPIO port number */
                    gpioModePushPull,         /* Pin mode is set to push pull */
                    1);                       /* High idle state */

    GPIO_PinModeSet(gpioPortD,                /* GPIO port */
                    5,                        /* GPIO port number */
                    gpioModeInputPull,         /* Pin mode is set to push pull */
                    1);                       /* High idle state */

    /* Enable LEUART interrupt vector in NVIC */
    NVIC_EnableIRQ(LEUART0_IRQn);
}

// Check for valid data, and if so clear it by pulling it out.
void LEUART0_IRQHandler(void) {
    /* Store and reset pending interupts */
    uint32_t leuartif = LEUART_IntGet(LEUART0);
    LEUART_IntClear(LEUART0, leuartif);
    count_leuart_irqs++;

    sIOBlockingData *s = &connection_state[0];

    if ( leuartif & LEUART_IEN_RXDATAV ) {
        ringbuffer_addchar(s->rb_rx, LEUART0->RXDATAX);
        // See if there is a blocking read
        if ( s->blocked_rx ) {
            s->blocked_rx = false;
            // If there is also a tx block, don't wake.
            // This should not happen, since first event is blocking.

            if ( s->tcb ) forth_thread_restart(s);
        }
    }

    // If we finished a transmission, check for more work.
    if ( leuartif & LEUART_IEN_TXC ) {

        // Check for pended XON/XOFF characters.
        if ( s->pended_fc_char ) {
            LEUART0->TXDATA = s->pended_fc_char;
            s->pended_fc_char = 0;
            return;
        }

        // Walk things in order to prioritize.
        for ( int i = 0; i < 2; i++) {
            s = &connection_state[i];
            int used = ringbuffer_used(s->rb_tx);
            if ( used ) {
                uint32_t thechar = ringbuffer_getchar(s->rb_tx);
                LEUART0->TXDATA = thechar;
                used--;

                // See if thats the last char, and if so, re-start.
                if ( used == 0 && \
                        s->blocked_tx == true ) {
                    s->blocked_tx = false;

                    if ( s->tcb ) forth_thread_restart(s);
                }
                return; // One char per customer!
            } // if ( used )
        } // for()
    } //  leuartif & LEUART_IEN_TXC

} // Handler

// ------------------------------------------------------------
// The console puchar() call.  Intended for use with forth
// Returns t/f based upon whether or not the write is a
// blocking event - the thread must yield.
// Note that accessing low energy peripherals is a slow process.
// Don't waste any operations.
// ------------------------------------------------------------
static uint32_t pended = 0;

bool console_leuart_putchar(int stream, int c,  unsigned long *tcb) {

    pended++;

    // Make sure that there is no syncing to the LE Domain going on.
    // If thats happening, we have no choice but to wait.
    while ( LEUART0->SYNCBUSY ) { ; }

    // Check for a flush.
    if ( c == -1 ) {
        console_leuart_spin();
        return(false);
    }

    // Condition #1 - There is already data in the TX FIFO.
    // or the FIFO is full.  Add it and check for highwater.
    uint32_t leuart_status = LEUART0->STATUS;

    bool tx_hw_empty = (leuart_status & LEUART_STATUS_TXBL) != 0;

    // Check for an empty HW FIFO and bypass the ring buffer.
    // This gets a bit complicated because we need to prioritize stream 0.
    if ( tx_hw_empty ) {
        // If the RBufs are empty, bypass it.
        if ( ringbuffer_used(connection_state[0].rb_tx) == 0 && \
                ringbuffer_used(connection_state[1].rb_tx) == 0 ) {
            LEUART0->TXDATA = c;
            return(false); // No need to block.
        }
        // We didn't bypass.   There is data someplace.
        for ( int i = 0; i < 2 ; i++ ) {
            if ( ringbuffer_used(connection_state[i].rb_tx) ) {
                int thechar = ringbuffer_getchar(connection_state[i].rb_tx);
                LEUART0->TXDATA = thechar;
                break;
            }
        }
    }

    // We've removed any existing data from existing FIFO.
    // Now add it to the correct FIFOs and then decide what to do.
    int free = ringbuffer_addchar(connection_state[stream].rb_tx, c);

    // If we're maxing out, tell the caller to yield.
    if ( free == 0 ) { // Let it fill up.  No flow control chars in the ringbuffer.
        connection_state[stream].tcb = tcb;
        connection_state[stream].blocked_tx = true;
        connection_state[stream].blocked_count_tx++;
        if ( tcb ) forth_thread_stop(&connection_state[stream]);
        return(true);
    } else return(false);
}

// ------------------------------------------------------------
// The console charsavailable() call for use with key?
// This will work fine for all streams, as there will only
// be characters available on stream 0.
// ------------------------------------------------------------
int console_leuart_charsavailable(int stream) {
    return( ringbuffer_used(connection_state[stream].rb_rx));
}

// ------------------------------------------------------------
// console getchar().   Return the next character or -1.
// Supports blocking reads.
// ------------------------------------------------------------
int console_leuart_getchar(int stream, unsigned long *tcb) {

    if ( stream != 0 ) return(false); // No support for reading from stream 1

    int result = ringbuffer_getchar(connection_state[stream].rb_rx);

    if ( result == -1 ) {
        connection_state[stream].tcb = tcb;
        connection_state[stream].blocked_rx = true;
        connection_state[stream].blocked_count_rx++;
        if ( tcb ) forth_thread_stop(&connection_state[stream]);
    }

    return(result);
}

// ------------------------------------------------------------
// console send eol.  This should really be a wrapped
// form of
// ------------------------------------------------------------
bool console_leuart_eol(unsigned long *tcb) {
    return(false);
}

// ------------------------------------------------------------
// ------------------------------------------------------------
// ------------------------------------------------------------
// ------------------------------------------------------------
// ------------------------------------------------------------
void console_leuart_init() {
    int i;
    for (i = 0; i <= 1; i++) {
        connection_state[i].rb_rx = &rb_rx[i];
        connection_state[i].rb_tx = &rb_tx[i];
        ringbuffer_init(connection_state[i].rb_rx, rb_storage_rx[i], RX_FIFOSIZE);
        ringbuffer_init(connection_state[i].rb_tx, rb_storage_tx[i], TX_FIFOSIZE);
    }
}

uint32_t console_leuart_probe(int stream) {
    // return( (uint32_t) &rb_tx);
    // return(pended);
    return(ringbuffer_used(connection_state[stream].rb_tx));
}
