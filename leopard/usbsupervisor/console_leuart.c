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

#define RX_FIFOSIZE 32
#define TX_FIFOSIZE 32

static uint8_t rb_storage_rx[RX_FIFOSIZE];
static uint8_t rb_storage_tx[TX_FIFOSIZE];

static RINGBUF rb_rx; 
static RINGBUF rb_tx; 

/* LEUART1 initialization data */
const LEUART_Init_TypeDef leuart0Init =
{
  .enable   = leuartEnable,       /* Activate data reception on LEUn_TX pin. */
  .refFreq  = 0,                    /* Inherit the clock frequenzy from the LEUART clock source */
  .baudrate = 9600,                 /* Baudrate = 9600 bps */
  .databits = leuartDatabits8,      /* Each LEUART frame containes 8 databits */
  .parity   = leuartNoParity,       /* No parity bits in use */
  .stopbits = leuartStopbits1,      /* Setting the number of stop bits in a frame to 2 bitperiods */
};

// ------------- Statistics ------------------
int count_leuart_irqs = 0;

void initLeuart(void)
{
  /* Reseting and initializing LEUART0 */
  LEUART_Reset(LEUART0);
  LEUART_Init(LEUART0, &leuart0Init);

  /* Route LEUART0 TX pin to DMA location 0 */
  LEUART0->ROUTE = LEUART_ROUTE_TXPEN | LEUART_ROUTE_RXPEN | LEUART_ROUTE_LOCATION_LOC0;

  /* Enable TX Completion and RX Data */
  // LEUART_IntEnable(LEUART0, LEUART_IEN_RXDATAV|LEUART_IEN_TXBL);

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
  // NVIC_EnableIRQ(LEUART0_IRQn);
  }

// Check for valid data, and if so clear it by pulling it out.
void LEUART0_IRQHandler(void) {
	/* Store and reset pending interupts */
	uint32_t leuartif = LEUART_IntGet(LEUART0);
 	LEUART_IntClear(LEUART0, leuartif);
  	count_leuart_irqs++;
  
	if ( leuartif & LEUART_IEN_RXDATAV ) {
		ringbuffer_addchar(&rb_rx, LEUART0->RXDATAX);
		// See if there is a blocking read 
 		}

	// It the FIFO emptied out, check for more work.
	if ( ( leuartif & LEUART_IEN_TXBL ) && ringbuffer_used(&rb_tx) ) {
		uint32_t thechar = ringbuffer_getchar(&rb_tx);
		LEUART0->TXDATA = thechar;
		}
		// Set if there is a blocked thread waiting for space.
	
	}

// ------------------------------------------------------------
// The console puchar() call.  Intended for use with forth
// Returns t/f based upon whether or not the write is a 
// blocking event.    
// Note that accessing low energy peripherals is a slow process.
// Don't waste any operations.
// ------------------------------------------------------------
bool console_leuart_putchar(int c) {
	
	// Condition #1 - There is already data in the TX FIFO.
	// or the FIFO is full.  Add it and check for highwater.
	
	uint32_t leuart_status = LEUART0->STATUS;
	
	bool tx_hw_empty = (leuart_status & LEUART_STATUS_TXBL) != 0;
	
	// Check for en empty HW FIFO and bypass the ring buffer.
	if ( tx_hw_empty ) {
		// If the RB is empty, bypass it. 
		if ( ringbuffer_used(&rb_tx) == 0) { 
			LEUART0->TXDATA = c;
			}
		// Otherwise there is something in there, and it needs
		// to be sent first.
		else { 
			int thechar = ringbuffer_getchar(&rb_tx);
			LEUART0->TXDATA = thechar;
			ringbuffer_addchar(&rb_tx,c); // Don't check status.  No change.
			}
		return(false);
		}
	
	// Otherwise the TX FIFO was full.
	int free = ringbuffer_addchar(&rb_tx,c);
	if ( free <= 1 ) { // Always reserve the last char for XOFF
		return(true);
		}
	else return(false);
	}


void console_leuart_init() {
	ringbuffer_init(&rb_rx,rb_storage_rx,RX_FIFOSIZE);
	ringbuffer_init(&rb_tx,rb_storage_tx,RX_FIFOSIZE);			
	}

void console_leuart_spin() {
	bool busy;
	do { 
		busy = (LEUART0->STATUS & LEUART_STATUS_TXBL) == 0 ;
		} while ( busy );	
	}
