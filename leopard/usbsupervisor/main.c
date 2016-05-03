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
#include "bsp.h"
#include "segmentlcd.h"
#include "bsp_trace.h"

#include "em_usb.h"
#include "msdd.h"
#include "cdc.h"
#include "descriptors.h"

#include "ringbuffer.h"

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

/**************************************************************************//**
 * @brief main - the entrypoint after reset.
 *****************************************************************************/

extern uint32_t app_start_address;
extern RINGBUF rb;
int main( void )
{
  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  CMU_ClockSelectSet( cmuClock_HF, cmuSelect_HFXO );
  CMU_OscillatorEnable(cmuOsc_LFXO, true, false);

  /* Initialize LCD driver */
  SegmentLCD_Init(false);
  SegmentLCD_Write("usbcomp");
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);

  /* Initialize LED driver */
  BSP_LedsInit();

  CDC_Init();                   /* Initialize the communication class device. */

  /* Initialize and start USB device stack. */
  USBD_Init(&usbInitStruct);

   // We need PendSV to launch forth.  Its always enabled.
   NVIC_SetPriority(PendSV_IRQn,7);

  /*
   * When using a debugger it is practical to uncomment the following three
   * lines to force host to re-enumerate the device.
   */
  /* USBD_Disconnect();         */
  /* USBTIMER_DelayMs( 1000 );  */
  /* USBD_Connect();            */

	// Make the one-way trip.
	
    // LaunchApp(0x20000);

	// char buf[20];
	// usprintf(buf,"%x",*((unsigned long *) (0xE000ED04) ));
	// SegmentLCD_Write(buf);

int count = 0;
int c = -1;
  for (;;) {
	count++;
	if ( count > 1000000) {
		count -= 1000000;
		c++;
		c %= 26;
		PutChar(10,c + 'A');
	}
	int c2 = GetChar(10);
	while ( c2 > 0 ) {
		PutChar(10,c2);
		PutChar(10,'!');
		c2 = GetChar(10);
		}
	}
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
