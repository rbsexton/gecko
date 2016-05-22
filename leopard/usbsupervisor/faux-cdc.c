/**************************************************************************//**
 * @file cdc.c
 * @brief USB Communication Device Class (CDC) driver.
 * @version 4.1.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/
#include <stdint.h>

#include "em_device.h"
#include "em_common.h"
#include "em_cmu.h"
#include "em_dma.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "em_usb.h"
#include "bsp.h"
#include "dmactrl.h"
#include "cdc.h"

#include "ringbuffer.h"

#define CHANNELS 2

RINGBUF rb_IN;
RINGBUF rb_OUT;

// We need to look up endpoint numbers. 
int ep_IN[CHANNELS];
int ep_OUT[CHANNELS];

// The most we'll buffer before a packet to the host.
// This should be big enough to hold all of the startup messages.
#define RBPAYLOAD_IN 128
uint8_t rb_storage_in[RBPAYLOAD_IN];

// This only needs to be big enough to guarantee room for a single packet 
// from the host.
#define RBPAYLOAD_OUT 64 
uint8_t rb_storage_out[RBPAYLOAD_OUT];

// When a client requests wakeup, we need to remember.
long *wake_OUT[1];
long *wake_IN[1];

// Counters.
unsigned count_usb_xmit_packets;
unsigned count_usb_xmit_bytes;

unsigned count_recv_packets;
unsigned count_recv_bytes;

unsigned count_syscalls_putchar[2];
unsigned count_syscalls_putstring[2];
unsigned count_syscalls_putstring_bytes[2];
unsigned count_syscalls_getchar[2];
unsigned count_syscalls_eol[2];


/**************************************************************************//**
 * @addtogroup Cdc
 * @{ Implements USB Communication Device Class (CDC).

@section cdc_intro CDC implementation.

   The source code of the CDC implementation resides in
   kits/common/drivers/cdc.c and cdc.h. This driver implements a basic
   USB to RS232 bridge.

@section cdc_config CDC device configuration options.

  This section contains a description of the configuration options for
  the driver. The options are @htmlonly #define's @endhtmlonly which are
  expected to be found in the application "usbconfig.h" header file.
  The values shown below are from the Giant Gecko DK3750 CDC example.

  @verbatim
// USB interface numbers. Interfaces are numbered from zero to one less than
// the number of concurrent interfaces supported by the configuration. A CDC
// device is by itself a composite device and has two interfaces.
// The interface numbers must be 0 and 1 for a standalone CDC device, for a
// composite device which includes a CDC interface it must not be in conflict
// with other device interfaces.
#define CDC_CTRL_INTERFACE_NO ( 0 )
#define CDC_DATA_INTERFACE_NO ( 1 )

// Endpoint address for CDC data reception.
#define CDC_EP_DATA_OUT ( 0x01 )

// Endpoint address for CDC data transmission.
#define CDC_EP_DATA_IN ( 0x81 )

// Endpoint address for the notification endpoint (not used).
#define CDC_EP_NOTIFY ( 0x82 )

// Timer id, see USBTIMER in the USB device stack documentation.
// The CDC driver has a Rx timeout functionality which require a timer.
#define CDC_TIMER_ID ( 0 )

// DMA related macros, select DMA channels and DMA request signals.
#define CDC_UART_TX_DMA_CHANNEL   ( 0 )
#define CDC_UART_RX_DMA_CHANNEL   ( 1 )
#define CDC_TX_DMA_SIGNAL         DMAREQ_UART1_TXBL
#define CDC_RX_DMA_SIGNAL         DMAREQ_UART1_RXDATAV

// UART/USART selection macros.
#define CDC_UART                  UART1
#define CDC_UART_CLOCK            cmuClock_UART1
#define CDC_UART_ROUTE            ( UART_ROUTE_RXPEN | UART_ROUTE_TXPEN | \
                                    UART_ROUTE_LOCATION_LOC2 )
#define CDC_UART_TX_PORT          gpioPortB
#define CDC_UART_TX_PIN           9
#define CDC_UART_RX_PORT          gpioPortB
#define CDC_UART_RX_PIN           10

// Activate the RS232 switch on DK's.
#define CDC_ENABLE_DK_UART_SWITCH() BSP_PeripheralAccess(BSP_RS232_UART, true)

// No RS232 switch on STK's. Leave the definition "empty".
#define CDC_ENABLE_DK_UART_SWITCH()

  @endverbatim
 ** @} ***********************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/*** Typedef's and defines. ***/

#define CDC_BULK_EP_SIZE  (USB_FS_BULK_EP_MAXSIZE) /* This is the max. ep size.    */
#define CDC_USB_RX_BUF_SIZ  CDC_BULK_EP_SIZE /* Packet size when receiving on USB. */
#define CDC_USB_TX_BUF_SIZ  127    /* Packet size when transmitting on USB.  */

/* Calculate a timeout in ms corresponding to 5 char times on current     */
/* baudrate. Minimum timeout is set to 10 ms.                             */
#define CDC_RX_TIMEOUT    EFM32_MAX(10U, 50000 / (cdcLineCoding.dwDTERate))

/* The serial port LINE CODING data structure, used to carry information  */
/* about serial port baudrate, parity etc. between host and device.       */
EFM32_PACK_START(1)
typedef struct
{
  uint32_t dwDTERate;               /** Baudrate                            */
  uint8_t  bCharFormat;             /** Stop bits, 0=1 1=1.5 2=2            */
  uint8_t  bParityType;             /** 0=None 1=Odd 2=Even 3=Mark 4=Space  */
  uint8_t  bDataBits;               /** 5, 6, 7, 8 or 16                    */
  uint8_t  dummy;                   /** To ensure size is a multiple of 4 bytes */
} __attribute__ ((packed)) cdcLineCoding_TypeDef;
EFM32_PACK_END()


/*** Function prototypes. ***/

static int  LineCodingReceived(USB_Status_TypeDef status,
                               uint32_t xferred,
                               uint32_t remaining);

static int UsbDataReceivedMeta(USB_Status_TypeDef status,
	uint32_t xferred, uint32_t remaining, int channel);
static int UsbDataTransmittedMeta(USB_Status_TypeDef status,
	uint32_t xferred, uint32_t remaining, int channel);

static int UsbDataReceivedU0(USB_Status_TypeDef status,
 		uint32_t xferred, uint32_t remaining) {		
	return(UsbDataReceivedMeta(status, xferred, remaining, 0));
	}

static int UsbDataTransmittedU0(USB_Status_TypeDef status,
	uint32_t xferred, uint32_t remaining ) {
		return(UsbDataTransmittedMeta(status,xferred,remaining,0));
	}

static void RingbufferSweep(void);
static void RingTXCheck(void);

#if 0 
static int UsbDataReceivedU1(USB_Status_TypeDef status,
 		uint32_t xferred, uint32_t remaining) {		
	return(UsbDataReceivedMeta(status, xferred, remaining, 1));
	}

static int UsbDataTransmittedU1(USB_Status_TypeDef status,
	uint32_t xferred, uint32_t remaining) {
		return(UsbDataTransmittedMeta(status,xferred,remaining,1));
	}
#endif 

/*** Variables ***/

/*
 * The LineCoding variable must be 4-byte aligned as it is used as USB
 * transmit and receive buffer.
 */
EFM32_ALIGN(4)
EFM32_PACK_START(1)
static cdcLineCoding_TypeDef __attribute__ ((aligned(4))) cdcLineCoding =
{
  115200, 0, 0, 8, 0
};
EFM32_PACK_END()

STATIC_UBUF(usbRxBuffer0,  CDC_USB_RX_BUF_SIZ);   /* USB receive buffers.   */
STATIC_UBUF(usbRxBuffer1,  CDC_USB_RX_BUF_SIZ);
STATIC_UBUF(usbTxBuffer0,  CDC_USB_TX_BUF_SIZ);   /* We only need one. */

static const uint8_t  *usbRxBuffer[  2 ] = { usbRxBuffer0, usbRxBuffer1 };

static int            usbRxIndex;

static bool           usbRxActive;
static volatile bool usbTxActive[2] = { false, false }; // Mediates access to tx buffers.

static bool clientAttached;
/** @endcond */

/**************************************************************************//**
 * @brief CDC device initialization.
 *****************************************************************************/
void CDC_Init( void )
{
	ep_IN[0] = CDC_EP_DATA_IN;
	ep_OUT[0] = CDC_EP_DATA_OUT;

	ringbuffer_init(&rb_IN,rb_storage_in,RBPAYLOAD_IN);
	ringbuffer_init(&rb_OUT,rb_storage_out,RBPAYLOAD_OUT);
}

/**************************************************************************//**
 * @brief
 *   Handle USB setup commands. Implements CDC class specific commands.
 *
 * @param[in] setup Pointer to the setup packet received.
 *
 * @return USB_STATUS_OK if command accepted.
 *         USB_STATUS_REQ_UNHANDLED when command is unknown, the USB device
 *         stack will handle the request.
 *****************************************************************************/
int CDC_SetupCmd(const USB_Setup_TypeDef *setup)
{
  int retVal = USB_STATUS_REQ_UNHANDLED;

  if ( ( setup->Type      == USB_SETUP_TYPE_CLASS          ) &&
       ( setup->Recipient == USB_SETUP_RECIPIENT_INTERFACE )    )
  {
    switch (setup->bRequest)
    {
    case USB_CDC_GETLINECODING:
      /********************/
      if ( ( setup->wValue    == 0                     ) &&
           ( setup->wIndex    == CDC_CTRL_INTERFACE_NO ) && /* Interface no. */
           ( setup->wLength   == 7                     ) && /* Length of cdcLineCoding. */
           ( setup->Direction == USB_SETUP_DIR_IN      )    )
      {
        /* Send current settings to USB host. */
        USBD_Write(0, (void*) &cdcLineCoding, 7, NULL);
        retVal = USB_STATUS_OK;
      }
      break;

    case USB_CDC_SETLINECODING:
	  // Happens on connect.  Apparently platform dependendant.
      /********************/
      if ( ( setup->wValue    == 0                     ) &&
           ( setup->wIndex    == CDC_CTRL_INTERFACE_NO ) && /* Interface no. */
           ( setup->wLength   == 7                     ) && /* Length of cdcLineCoding. */
           ( setup->Direction != USB_SETUP_DIR_IN      )    )
      {
        /* Get new settings from USB host. */
        USBD_Read(0, (void*) &cdcLineCoding, 7, LineCodingReceived);
        retVal = USB_STATUS_OK;
      }
      // BSP_LedsSet(BSP_LedsGet() | 0x2 );
	  clientAttached = true;
	  break;

    case USB_CDC_SETCTRLLINESTATE:
	  // Apparently this will trigger on disconnect.
      /********************/
      if ( ( setup->wIndex  == CDC_CTRL_INTERFACE_NO ) &&   /* Interface no.  */
           ( setup->wLength == 0                     )    ) /* No data.       */
      {
        /* Do nothing ( Non compliant behaviour !! ) */
        retVal = USB_STATUS_OK;
      }
	  // BSP_LedsSet(0);
	  clientAttached = false;
      break;
    }
  }

  return retVal;
}

/**************************************************************************//**
 * @brief
 *   Callback function called when the data stage of a CDC_SET_LINECODING
 *   setup command has completed.
 *
 * @param[in] status    Transfer status code.
 * @param[in] xferred   Number of bytes transferred.
 * @param[in] remaining Number of bytes not transferred.
 *
 * @return USB_STATUS_OK if data accepted.
 *         USB_STATUS_REQ_ERR if data calls for modes we can not support.
 *****************************************************************************/
static int LineCodingReceived(USB_Status_TypeDef status,
                              uint32_t xferred,
                              uint32_t remaining)
{
  uint32_t frame = 0;
  (void) remaining;

  /* We have received new serial port communication settings from USB host. */
  if ((status == USB_STATUS_OK) && (xferred == 7))
  {
    return USB_STATUS_OK;
  }
  return USB_STATUS_REQ_ERR;
}

/**************************************************************************//**
 * @brief
 *   Callback function called each time the USB device state is changed.
 *   Starts CDC operation when device has been configured by USB host.
 *
 * @param[in] oldState The device state the device has just left.
 * @param[in] newState The new device state.
 *****************************************************************************/
void CDC_StateChangeEvent( USBD_State_TypeDef oldState,
                           USBD_State_TypeDef newState)
{
  if (newState == USBD_STATE_CONFIGURED)
  {
    /* We have been configured, start CDC functionality ! */

    if (oldState == USBD_STATE_SUSPENDED)   /* Resume ?   */
    {
    }

    /* Start receiving data from USB host. */
    usbRxIndex  = 0;
    usbRxActive = true;
    USBD_Read(ep_OUT[0], (void*) usbRxBuffer[ usbRxIndex ],
              CDC_USB_RX_BUF_SIZ, UsbDataReceivedU0);
	// Don't trigger a send just yet.   The client may not be ready.
	// USBTIMER_Start(CDC_TIMER_ID, 5, RingbufferSweep);
  }

  else if ((oldState == USBD_STATE_CONFIGURED) &&
           (newState != USBD_STATE_SUSPENDED))
  {
    /* We have been de-configured, stop CDC functionality. */
    USBTIMER_Stop(CDC_TIMER_ID);
    /* Stop DMA channels. */
  }

  else if (newState == USBD_STATE_SUSPENDED)
  {
    /* We have been suspended, stop CDC functionality. */
    /* Reduce current consumption to below 2.5 mA.     */
    // USBTIMER_Stop(CDC_TIMER_ID);
  }
}

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/**************************************************************************//**
 * @brief Callback function called whenever a new packet with data is received
 *        on USB.
 *
 * @param[in] status    Transfer status code.
 * @param[in] xferred   Number of bytes transferred.
 * @param[in] remaining Number of bytes not transferred.
 *
 * @return USB_STATUS_OK.
 *****************************************************************************/
static int UsbDataReceivedMeta(USB_Status_TypeDef status,
                           uint32_t xferred,
                           uint32_t remaining, int channel)
{
  (void) remaining;            /* Unused parameter. */
  (void) channel;              /* Not used yet. */

  if ((status == USB_STATUS_OK) && (xferred > 0))
  {

	// Put the data into the ringbuffer.  This could also be done with bulk.
	
	RINGBUF *rb = &rb_OUT;
	int i=0;
	if ( xferred < ringbuffer_free(rb)) {
		while ( xferred-- ) ringbuffer_addchar(rb, usbRxBuffer[usbRxIndex][i++]);
		}
	
	// See if there is a wake request
	if ( wake_OUT[0] ) {
		*wake_OUT[0] |= 1; // Set the run bit.
		wake_OUT[0] = 0;
		}	
	// uint32_t i;
	
	// USBD_Write(CDC_EP_DATA_IN, (void*) usbRxBuffer[ usbRxIndex ],
    //           xferred, UsbDataTransmittedU0);

	// Now we decide whether or not to request another packet the 
	// next time we get polled.    
    usbRxIndex ^= 1; // Switch to the other one.
    /* Start a new USB receive transfer. */
    USBD_Read(ep_OUT[0], (void*) usbRxBuffer[ usbRxIndex ],
              CDC_USB_RX_BUF_SIZ, UsbDataReceivedU0);

	// int leds = BSP_LedsGet();
	// leds++;
	// BSP_LedsSet(leds);
  }
  return USB_STATUS_OK;
}

static void SendRBtoHost() {
	if ( ! clientAttached ) return;

	int i = 0;
 	while( ringbuffer_used(&rb_IN) ) {
		usbTxBuffer0[i++] = ringbuffer_getchar(&rb_IN);
		}
	if ( i ) {
		usbTxActive[0] = true;
		USBD_Write(ep_IN[0], (void*) usbTxBuffer0, i, UsbDataTransmittedU0);
		count_usb_xmit_packets++;
		count_usb_xmit_bytes += i;		
		}
		// strcpy(usbTxBuffer0,"Foo");
	// USBD_Write(CDC_EP_DATA_IN, (void*) usbTxBuffer0, 3, UsbDataTransmittedU0);
	
	
	}

// This needs to be re-named.   This will be the on-demand timer task 
// That grabs larger chunks of stuff.
static int sweep_counter = 0;
static void RingbufferSweep(void) { 

	if (usbTxActive[0]) {
		BSP_LedsSet(BSP_LedsGet() | 0x2 );     
	} else {
		BSP_LedsSet(0);     
		}
	sweep_counter += 10;
	
	if ( (sweep_counter > 1000) && !usbTxActive[0] ) {
		sweep_counter -= 1000;
		// int leds = BSP_LedsGet();
		// leds++;
		// BSP_LedsSet(leds);
		SendRBtoHost();
		}
	// We have to re-call ourself.
	// USBTIMER_Start(CDC_TIMER_ID, 10, RingbufferSweep);
	}

/**************************************************************************//**
 * @brief Callback function called whenever a packet with data has been
 *        transmitted on USB
 *
 * @param[in] status    Transfer status code.
 * @param[in] xferred   Number of bytes transferred.
 * @param[in] remaining Number of bytes not transferred.
 *
 * @return USB_STATUS_OK.
 *****************************************************************************/
static int UsbDataTransmittedMeta(USB_Status_TypeDef status,
                              uint32_t xferred,
                              uint32_t remaining,
							  int channel)
{
  (void) xferred;              /* Unused parameter. */
  (void) remaining;            /* Unused parameter. */
  (void) channel;              /* As of yet parameter. */

  usbTxActive[0] = false;

  if (status == USB_STATUS_OK) {
	RingTXCheck(); // Look for more work.	  
    // USBTIMER_Start(CDC_TIMER_ID, CDC_RX_TIMEOUT, UartRxTimeout);
    }
  return USB_STATUS_OK;
}


/**************************************************************************//**
Now for the offical system calls.
 *****************************************************************************/

static void RingTxBite(RINGBUF *rb, volatile bool *txflag) {
	int i;
	int used = ringbuffer_used(rb);
	if ( used > 64 ) used = 64; // Set a max size.
	for(i=0; i < used; i++) usbTxBuffer0[i] = ringbuffer_getchar(rb);
	*txflag = true;
	count_usb_xmit_packets++;
	count_usb_xmit_bytes += i;			
	USBD_Write(ep_IN[0], (void*) usbTxBuffer0, i, UsbDataTransmittedU0);
	}

// The most effient way to handle things is to do a transmission
// when the outgoing ringbuffer starts getting full, or after 
// a timeout. This routine is our callback. 
static void RingTXCheck(void) {
	RINGBUF *rb = &rb_IN;
	volatile bool *active = &usbTxActive[0];
		
	int used = ringbuffer_used(rb);
	if ( used ) { 
		RingTxBite(rb,active);
		}
	}

static void CheckAndSend(int free) {
	if ( clientAttached ) {
		// If the ringbuffer is full, go ahead and trigger.
		if ( free == 0 )  RingTXCheck();
		else USBTIMER_Start(CDC_TIMER_ID, 10, RingTXCheck);
		}
	}
	
int USBPutChar(int usbstream, uint8_t c) {
	count_syscalls_putchar[usbstream]++;
	
	RINGBUF *rb = &rb_IN;
	volatile bool *active = &usbTxActive[usbstream];
	if ( *active ) { // If there is xmission going on...
		return(ringbuffer_addchar(rb,c));
		}
		
	// Go Ahead and put it into the buffer.
	// Scenarios 
	// 1 - There was room.   Call txbite.
	// 2 - There was not room.
	int free = ringbuffer_addchar(rb,c);
	CheckAndSend(free);
	return(free);
	}

// Implement the SAPI Calls.
uint32_t USBGetChar(uint32_t usbstream, long *tcb) {
	count_syscalls_getchar[usbstream]++;
	RINGBUF *rb = &rb_OUT;
	if ( ringbuffer_used(rb) ) return(ringbuffer_getchar(rb));
	else {
		if ( tcb ) {
			wake_OUT[0] = tcb + 2; // Go ahead and correct the pointer.
			tcb[2] &= ~1; // Clear the run bit
			}
		return(-1);
		}
	}

int USBPutString(int usbstream, int len, uint8_t *p,  unsigned long *tcb) {
	count_syscalls_putstring[usbstream]++;
	count_syscalls_putstring_bytes[usbstream] += len;
	
	RINGBUF *rb = &rb_IN;
	int free = ringbuffer_free(rb);
	
	if ( len > free ) len = free; // Don't over-flow the buffer.
	free -= len; // Pre-account for the space that we'll use.
	int ret = len;   // This is what we'll return.
	
	while( len-- ) {
		ringbuffer_addchar(rb,*p++);
		}

	// In any case, we'll want to try and send it out.
	volatile bool *active = &usbTxActive[usbstream];
	if ( *active == false ) { // If there is no xmission going on.
		CheckAndSend(free);
		}
	return(ret);	
	}


int USBOutEOL(uint32_t usbstream, long *tcb) {
	count_syscalls_eol[usbstream]++;

	RINGBUF *rb = &rb_IN;
	int free = ringbuffer_free(rb);
	if  ( free >= 2 ) {
		ringbuffer_addchar(rb,13);
		ringbuffer_addchar(rb,10); // Don't use magic C characters!
		free -= 2;
		}
	else { // If the buffer is full, somebody is working on things.
		// if ( tcb ) {
		//	wake_IN[0] = tcb + 2; // Go ahead and correct the pointer.
		//	tcb[2] &= ~1; // Clear the run bit
		//	}
		return(0);
		}
	
	volatile bool *active = &usbTxActive[usbstream];
	if ( *active == false ) { // If there is no xmission going on.
		CheckAndSend(free);
		}
	return(1);
	}

uint32_t USBGetCharAvail(uint32_t usbstream) {
	RINGBUF *rb = &rb_OUT;	
	return( ringbuffer_used(rb));
	}





/** @endcond */
