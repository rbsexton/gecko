/**************************************************************************//**
 * @file main.c
 * @brief USB Composite Device example.
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
#include <stdio.h>
#include "em_device.h"
#include "em_assert.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_leuart.h"
#include "em_rtc.h"

#include "bsp.h"
#include "segmentlcd.h"
#include "bsp_trace.h"

#include "em_usb.h"
#include "msdd.h"
#include "cdc.h"
#include "descriptors.h"

#include "ringbuffer.h"
#include "testsyscall.h"

#include "console_leuart.h"

// For the Leopard Gecko board, TX=PD4, RX=PD5

static const char lut[] = "0123456789ABCDEF";

// The Display only has 7 digits!
char hexcode[8] = "0123456"; // Make it null-terminated

static char *tohex(uint32_t in) {
	in = in & 0x0FFFFFFF; // Mask off the top nibble.
	int i;
	for (i = 0; i < 6; i++) {
		hexcode[6-i] = lut[ in & 0xF ];
		in = in>>4;
		}
	return(hexcode);
	}


/**************************************************************************//**
 *
 * This example shows how a Composite USB Device can be implemented.
 *
 *****************************************************************************/

int SetupCmd(const USB_Setup_TypeDef *setup);
void StateChangeEvent( USBD_State_TypeDef oldState,
                       USBD_State_TypeDef newState );

static const USBD_Callbacks_TypeDef callbacks =
{
  .usbReset        = NULL,
  .usbStateChange  = StateChangeEvent,
  .setupCmd        = SetupCmd,
  .isSelfPowered   = NULL,
  .sofInt          = NULL
};

const USBD_Init_TypeDef usbInitStruct =
{
  .deviceDescriptor    = &USBDESC_deviceDesc,
  .configDescriptor    = USBDESC_configDesc,
  .stringDescriptors   = USBDESC_strings,
  .numberOfStrings     = sizeof(USBDESC_strings)/sizeof(void*),
  .callbacks           = &callbacks,
  .bufferingMultiplier = USBDESC_bufferingMultiplier,
  .reserved            = 0
};

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// Boot Message
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
const char message[] = "Boot! ";
void SayHello() {
	const char *p = message;
	while(*p) LEUART_Tx(LEUART0,*p++);

	// console_leuart_spin();
	// Send it a second time via the buffered IO system
	p = message;
	while(*p) {
		console_leuart_putchar(0,*p, 0);
		p++;
		}
	}



// Exercise the SAPI system call interface.

void Syscall_Exercize(int stream) {
	const char *p = "Syscall! ";
	while(*p) PutChar(stream,*p++);
		
	int count = 0;
	int c = -1;
	  for (;;) {
		count++;
		if ( count > 100000) {
			count -= 100000;
			c++;
			c %= 26;
			PutChar(stream,c + 'A');
			}
			
		int c2 = GetCharAvail(stream);
		if ( c2 == 0 ) {
			__asm("wfi");
		} else {
			c2 = GetChar(stream);
			PutChar(stream,c2);
			PutChar(stream,'!');
			}
		}
	}



/**************************************************************************//**
 * @brief  Setup Real Time Clock (RTC)
 *
 * Intialize the RTC and use it to generate a 128Hz timebase.
 *
 *****************************************************************************/
/* Set up RTC init struct*/
const RTC_Init_TypeDef rtcInit =
{
  .debugRun = false,
  .comp0Top = true,
  .enable   = true,
};

void setupRtc(void)
{
  /* Input RTC init struct in initialize function */
  RTC_Init(&rtcInit);

  /* Set RTC compare value */
  // rtcCountBetweenWakeup = ((SystemLFXOClockGet() * WAKEUP_INTERVAL_MS) / 1000);
  RTC_CompareSet(0, 255);

  /* Enable RTC interrupt from COMP0 */
  RTC_IntEnable(RTC_IF_COMP0);

  /* Enable RTC interrupt vector in NVIC */
  NVIC_EnableIRQ(RTC_IRQn);

  /* Enable RTC */
  RTC_Enable(true);
}

/**************************************************************************//**
 * @brief RTC Interrupt Handler.
 *
 * Interrupt at 128Hz and keep track of time.
 * Produce a RTC item with 8 fractional bits.   That gives us about a year.
 *
 *****************************************************************************/
uint32_t rtc_time;
uint32_t rtc_ms;

void RTC_IRQHandler(void) {
	static int err = 52; // Running error for the interpolator.

	/* Clear interrupt source */
	RTC_IntClear(RTC_IFC_COMP0);
	rtc_time += 2;
	
	// Interpolater - 
	unsigned bump = 7;  // At least 7ms have passed.
	// Num = 104, den = 128
	err -= 104;
	if ( err < 0 ) {
		err += 128; 
		bump++;
		}
	rtc_ms += bump;
    }

/**************************************************************************//**
 * @brief main - the entrypoint after reset.
 *****************************************************************************/

extern uint32_t app_start_address;
extern RINGBUF rb;
extern uint32_t console_leuart_probe(); 

int main( void )
{
	
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO );
  CMU_OscillatorEnable(cmuOsc_LFXO, true, false); 

  /* Start LFXO, and use LFXO for low-energy modules */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

  CMU_ClockEnable(cmuClock_CORELE, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_LEUART0, true);    /* Enable LEUART0 clock */
  CMU_ClockEnable(cmuClock_RTC, true);        /* Enable RTC clock */

  console_leuart_init(); // Console Data structures.
  initLeuart();
 
  SayHello();

  /* Initialize LCD driver */
  SegmentLCD_Init(false);

  // uint32_t leuart_status = LEUART0->STATUS;
  // uint32_t leuart_status = CMU->CTRL;
  // uint32_t leuart_status = CMU->LFCLKSEL;
  uint32_t leuart_status = CMU->STATUS;
  // uint32_t leuart_status = CMU_ClockSelectGet(cmuClock_LFB);
  // uint32_t leuart_status = LEUART0->ROUTE;
  // uint32_t leuart_status = LEUART0->SYNCBUSY;
  // uint32_t leuart_status = LEUART0->CTRL;


  SegmentLCD_Write(tohex(*((uint32_t *) 0x20004)) );
  // SegmentLCD_Write(tohex(0x123A5678));
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);

  setupRtc();

  // Syscall_Exercize(0); 

  /* Initialize LED driver */
  BSP_LedsInit();

  CDC_Init();                   /* Initialize the communication class device. */

  /* Initialize and start USB device stack. */
  USBD_Init(&usbInitStruct);

  // We need PendSV to launch forth.  Its always enabled.
  NVIC_SetPriority(PendSV_IRQn,7);
  LaunchApp(0x20000);

  /*
   * When using a debugger it is practical to uncomment the following three
   * lines to force host to re-enumerate the device.
   */
  /* USBD_Disconnect();         */
  /* USBTIMER_DelayMs( 1000 );  */
  /* USBD_Connect();            */

	// Make the one-way trip.
	
  
	// char buf[20];
	// usprintf(buf,"%x",*((unsigned long *) (0xE000ED04) ));
	// SegmentLCD_Write(buf);

 }

/**************************************************************************//**
 * @brief
 *   Called whenever a USB setup command is received.
 *
 * @param[in] setup
 *   Pointer to an USB setup packet.
 *
 * @return
 *   An appropriate status/error code. See USB_Status_TypeDef.
 *****************************************************************************/
int SetupCmd(const USB_Setup_TypeDef *setup)
{
  int retVal;

  /* Call SetupCmd handlers for all functions within the composite device. */
  retVal = CDC_SetupCmd( setup );

  return retVal;
}

/**************************************************************************//**
 * @brief
 *   Called whenever the USB device has changed its device state.
 *
 * @param[in] oldState
 *   The device USB state just leaved. See USBD_State_TypeDef.
 *
 * @param[in] newState
 *   New (the current) USB device state. See USBD_State_TypeDef.
 *****************************************************************************/
void StateChangeEvent( USBD_State_TypeDef oldState,
                       USBD_State_TypeDef newState )
{
  /* Call device StateChange event handlers for all relevant functions within
     the composite device. */
  CDC_StateChangeEvent(  oldState, newState );
}
