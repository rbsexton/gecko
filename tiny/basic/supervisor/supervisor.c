/**************************************************************************//**
 * @file
 * @brief Simple LED Blink Demo for EFM32TG_STK3300
 * @version 3.20.5
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_leuart.h"
#include "em_rtc.h"

#include "bsp.h"
#include "segmentlcd.h"
#include "segmentlcdconfig.h"

#include "bl_launcher.h"
#include "interconnect.h"
#include "console_leuart.h"

// ------------- Statistics ------------------

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// RTC Code.
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
uint32_t rtcCountBetweenWakeup;

/* Set up RTC init struct*/
const RTC_Init_TypeDef rtcInit =
{
  .debugRun = true,
  .comp0Top = false,
  .enable   = true,
};

void setupRTC(void)
{
  /* Input RTC init struct in initialize funciton */
  RTC_Init(&rtcInit);

  /* Set RTC compare value */
  // rtcCountBetweenWakeup = 32768;
  // RTC_CompareSet(0, rtcCountBetweenWakeup);

  /* Enable RTC interrupt from COMP0 */
  // RTC_IntEnable(RTC_IF_COMP0);

  /* Enable RTC interrupt vector in NVIC */
  // NVIC_EnableIRQ(RTC_IRQn);

  /* Enable RTC */
  RTC_Enable(true);
}

void RTC_IRQHandler(void)
{
  /* Clear interrupt source */
  RTC_IntClear(RTC_IFC_COMP0);
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// Boot Message
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
extern bool PutChar(int, int, uint32_t );
const char message[] = "Boot! ";
void SayHello() {
	const char *p = message;
	while(*p) {
		// LEUART_Tx(LEUART0,*p++);
		// console_leuart_putchar(*p++,0);
		PutChar(0,*p++,0); // No TCB.
		}
	}


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void) __attribute ((noreturn)); 


int main(void)
{
  /* Chip errata */
  CHIP_Init();

  CMU_ClockEnable(cmuClock_HFPER, true);
 
  /* Start LFXO, and use LFXO for low-energy modules */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO); // RTC
  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO); // LEUART


  /* Enabling clocks, all other remain disabled */
  CMU_ClockEnable(cmuClock_CORELE, true);     /* Enable CORELE clock */
  CMU_ClockEnable(cmuClock_GPIO, true);       /* Enable GPIO clock */
  CMU_ClockEnable(cmuClock_LEUART0, true);    /* Enable LEUART0 clock */
  CMU_ClockEnable(cmuClock_LCD, true);        /* Enable LCD clock */

  CMU_ClockEnable(cmuClock_RTC, true);        /* Enable RTC clock */

  // Setup the LFB Clock dividers.
  CMU->LFAPRESC0 = (11 << _CMU_LFAPRESC0_RTC_SHIFT);

	setupRTC();
	
	/* Initialize LCD driver */
  	SegmentLCD_Init(false);
    SegmentLCD_Write("Hello2");

  /* Initialize LED driver */
  // BSP_LedsInit(); // This appears to be bloatware.

  /* Re-config the HFRCO to the low band */
  CMU_HFRCOBandSet(cmuHFRCOBand_14MHz); 

  /* Initialize LEUART */
  initLeuart();
  console_leuart_init();

  SayHello();


  // An Ugly, Bare loop to zero Forth's memory.
  memset((uint32_t *) 0x20000400, 0, (0x20000000 + 4096) - 0x20000400);

  // Let Forth set its own stack pointer.
  LaunchUserAppNoSP( (long unsigned int *) 0x2000, (uint32_t *) &theshareddata);

  /* Infinite blink loop */
  while (1)
  {
    // BSP_LedToggle(0);
	int i;
	for ( i = 0; i < 10000; i++ ) { ; }
	//Delay(100);
  }

}
