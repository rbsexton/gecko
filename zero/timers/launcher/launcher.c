/// *************************************************************************
/// * @file
/// * @brief Timer Drivers
/// *************************************************************************

#include "em_chip.h"
#include "em_device.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_leuart.h"
#include "em_dma.h"
#include "em_gpio.h"
#include "em_rtc.h"
#include "em_timer.h"

#include "bl_launcher.h"
#include "interconnect.h"

/* DEFINES */
#define WAKEUP_INTERVAL_MS    5000

/* GLOBAL VARIABLES */

uint32_t rtcCountBetweenWakeup;

/* Defining the LEUART1 initialization data */
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

// ******************* Boot Banner ****************************
const char message[] = "Boot! ";
void SayHello() {
	const char *p = message;
	while(*p) LEUART_Tx(LEUART0,*p++);
	}
	
/**************************************************************************//**
 * @brief  Initialize Low Energy UART 1
 *
 * Here the LEUART is initialized with the chosen settings. It is then routed
 * to location 0 to avoid conflict with the LCD pinout. Finally the GPIO mode
 * is set to push pull.
 *
 *****************************************************************************/

// For the Zero Gecko board, TX=PD4, RX=PD5

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

  /* Enable RTC interrupt vector in NVIC */
  NVIC_EnableIRQ(LEUART0_IRQn);

}

// Check for valid data, and if so clear it by pulling it out.
void LEUART0_IRQHandler(void) {
	/* Store and reset pending interupts */
	uint32_t leuartif = LEUART_IntGet(LEUART0);
 	LEUART_IntClear(LEUART0, leuartif);
  
	if ( leuartif & LEUART_IEN_RXDATAV ) {
		theshareddata.u0rxdata = LEUART0->RXDATAX;
	}
  	count_leuart_irqs++;
}

/**************************************************************************//**
 * @brief  Setup Real Time Clock (RTC)
 *
 * The RTC is initialized, and is set up to generate an interrupt every other
 * second.
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
  rtcCountBetweenWakeup = ((SystemLFXOClockGet() * WAKEUP_INTERVAL_MS) / 1000);
  RTC_CompareSet(0, rtcCountBetweenWakeup);

  /* Enable RTC interrupt from COMP0 */
  RTC_IntEnable(RTC_IF_COMP0);

  /* Enable RTC interrupt vector in NVIC */
  NVIC_EnableIRQ(RTC_IRQn);

  /* Enable RTC */
  RTC_Enable(true);
}

// See if there is a character in the LEUART buffer and echo it back.

void CheckandEcho() {
	if ( LEUART0->STATUS & LEUART_STATUS_RXDATAV ) {
		LEUART0->TXDATA = LEUART0->RXDATA;
		
		// LEUART_Tx(LEUART0,LEUART0->RXDATA);
	}
}

/**************************************************************************//**
 * @brief RTC Interrupt Handler.
 *
 * This routine will run every other second, when the RTC times out, and generate
 * an interrupt. After clearing the interrupt source, the DMA source address is
 * set, and a new DMA transfer is initiated. When the routine is finished, the
 * system will again enter EM2 while the DMA continues to transfer data from the
 * memory to the LEUART. The LEUART DMA wake-up on TX is enabled to allow a 
 * LEUART DMA request to wake up the DMA. 
 *
 *****************************************************************************/
void RTC_IRQHandler(void)
{
  /* Clear interrupt source */
  RTC_IntClear(RTC_IFC_COMP0);

  if(GPIO_PinInGet(gpioPortC, 10)) 
    GPIO_PinOutSet(gpioPortC, 10);   /* Drive high PD8 */ 
  else 
    GPIO_PinOutClear(gpioPortC, 10); /* Drive low PD8 */

}


/**************************************************************************//**
 * @brief Initialize the timer.
 * 
 * THe zero gecko board has Timer1 CC1 on Pin4/PD7 - Location 4.
 *****************************************************************************/
const TIMER_Init_TypeDef timerInit =
 {
   .enable     = true,
   .debugRun   = true,
   .prescale   = timerPrescale1,
   .clkSel     = timerClkSelHFPerClk,
   .fallAction = timerInputActionNone,
   .riseAction = timerInputActionNone,
   .mode       = timerModeUp,
   .dmaClrAct  = false,
   .quadModeX4 = false,
   .oneShot    = false,
   .sync       = false,
 };

const TIMER_InitCC_TypeDef timerCCInit = 
{
  .eventCtrl  = timerEventEveryEdge,
  .edge       = timerEdgeBoth,
  .prsSel     = timerPRSSELCh0,
  .cufoa      = timerOutputActionNone,
  .cofoa      = timerOutputActionSet,
  .cmoa       = timerOutputActionClear,
  .mode       = timerCCModePWM,
  .filter     = false,
  .prsInput   = false,
  .coist      = false,
  .outInvert  = false,
};

void setupTimers() {
  /* Set CC0 location 3 pin (PD1) as output */
  GPIO_PinModeSet(gpioPortD, 7, gpioModePushPull, 0);
  
  /* Configure CC channel 0 */
  TIMER_InitCC(TIMER1, 1, &timerCCInit);

  /* Route CC1 to location 3 (PD1) and enable pin */  
  TIMER1->ROUTE |= (TIMER_ROUTE_CC1PEN | TIMER_ROUTE_LOCATION_LOC4); 
  
  TIMER_TopSet(TIMER1, 16384);  /* Set Top Value */
  TIMER_CompareBufSet(TIMER1, 1, 8192);  /* Set compare value  */
  
  /* Configure timer */
  TIMER_Init(TIMER1, &timerInit);
}

/**************************************************************************//**
 * @brief Initialize the GPIOs.
 * 
 * THe zero gecko board has LEDs on PC10 & PC11
 *****************************************************************************/
void setupGPIO() {	
  /* Configure PC10 as push pull output */
  GPIO_PinModeSet(gpioPortC, 10, gpioModePushPullDrive, 0);
}


/**************************************************************************//**
 * @brief  Main function
 *
 * This example demonstrates a way to use the Low Energy UART to maintain full
 * UART communication capabilities, while spending a great majority of the time
 * in deep sleep mode EM2. The LEUART is in this example driven by the LFXO,
 * which provide good accuracy while consuming only small amounts of energy. In
 * addition the DMA is set up to read the data to be transmitted by the LEUART
 * directly from the system memory. This relieves the CPU from doing anything
 * other than initializing the transfer, and handle possible interrupts triggered
 * when the transfer is finished. In this case the strings "HELLO" and "THERE"
 * are alternatingly transmitted every other second through the LEUART.
 *
 *****************************************************************************/
int main(void)
{

  theshareddata.jumptable = (uint32_t) &jumptable; // Initialize it at runtime 

  /* Initialize chip */
  CHIP_Init();

  /* Start LFXO, and use LFXO for low-energy modules */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

  /* Enabling clocks, all other remain disabled */
  CMU_ClockEnable(cmuClock_CORELE, true);     /* Enable CORELE clock */
  CMU_ClockEnable(cmuClock_GPIO, true);       /* Enable GPIO clock */
  CMU_ClockEnable(cmuClock_LEUART0, true);    /* Enable LEUART0 clock */
  CMU_ClockEnable(cmuClock_RTC, true);        /* Enable RTC clock */
  CMU_ClockEnable(cmuClock_TIMER1, true);
   
  /* Re-config the HFRCO to the low band */
  CMU_HFRCOBandSet(cmuHFRCOBand_1MHz); 

  /* Initialize LEUART */
  initLeuart();

  /* Setup RTC as interrupt source */
  	setupRtc();
	setupTimers();
	setupGPIO();

SayHello();

GPIO_PinOutSet(gpioPortC, 10);   /* Drive high PD8 */ 

LaunchUserAppNoNVIC( (long unsigned int *) 0x2000);

  // Lets just do echo.
  while (1)
  {
	CheckandEcho();
    /* Enable deep sleep */
    // EMU_EnterEM2(false);
  }
}
