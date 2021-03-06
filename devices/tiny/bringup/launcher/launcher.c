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
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_leuart.h"
#include "em_rtc.h"

#include "bsp.h"

#include "bl_launcher.h"
#include "interconnect.h"

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// LEUART Code.
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
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

// The Tiny Gecko board also has the LEUART on PD4 & PD5
void initLeuart(void)
{
  /* Reseting and initializing LEUART1 */
  LEUART_Reset(LEUART0);
  LEUART_Init(LEUART0, &leuart0Init);

  /* Route LEUART0 TX pin to DMA location 0 */
  LEUART0->ROUTE = LEUART_ROUTE_TXPEN | LEUART_ROUTE_RXPEN |
                   LEUART_ROUTE_LOCATION_LOC0;

  /* Enable TX Completion and RX Data */
  LEUART_IntEnable(LEUART0, LEUART_IEN_RXDATAV);

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

// Clear the IRQ.  Forth will wake and collect the data.
void LEUART0_IRQHandler(void) {
	/* Store and reset pending interupts */
	uint32_t leuartif = LEUART_IntGet(LEUART0);
 	LEUART_IntClear(LEUART0, leuartif);
  
	if ( leuartif & LEUART_IEN_RXDATAV ) {
		theshareddata.u0rxdata = LEUART0->RXDATAX;
		// LEUART0->TXDATA = theshareddata.u0rxdata; // An echo check.
	}
  	count_leuart_irqs++;
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// RTC Code.
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
uint32_t rtcCountBetweenWakeup;

/* Set up RTC init struct*/
const RTC_Init_TypeDef rtcInit =
{
  .debugRun = false,
  .comp0Top = true,
  .enable   = true,
};

void setupRtc(void)
{
  /* Input RTC init struct in initialize funciton */
  RTC_Init(&rtcInit);

  /* Set RTC compare value */
  rtcCountBetweenWakeup = 32768;
  RTC_CompareSet(0, rtcCountBetweenWakeup);

  /* Enable RTC interrupt from COMP0 */
  RTC_IntEnable(RTC_IF_COMP0);

  /* Enable RTC interrupt vector in NVIC */
  // NVIC_EnableIRQ(RTC_IRQn);

  /* Enable RTC */
  // RTC_Enable(true);
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
const char message[] = "Boot! ";
void SayHello() {
	const char *p = message;
	while(*p) LEUART_Tx(LEUART0,*p++);
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
  // CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

  /* Enabling clocks, all other remain disabled */
  CMU_ClockEnable(cmuClock_CORELE, true);     /* Enable CORELE clock */
  CMU_ClockEnable(cmuClock_GPIO, true);       /* Enable GPIO clock */
  CMU_ClockEnable(cmuClock_LEUART0, true);    /* Enable LEUART0 clock */
  // CMU_ClockEnable(cmuClock_RTC, true);        /* Enable RTC clock */

  /* Initialize LED driver */
  // BSP_LedsInit(); // This appears to be bloatware.

  /* Re-config the HFRCO to the low band */
  CMU_HFRCOBandSet(cmuHFRCOBand_1MHz); 

  /* Initialize LEUART */
  initLeuart();

  SayHello();

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
