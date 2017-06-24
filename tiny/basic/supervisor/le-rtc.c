#include <stdio.h>
#include "em_device.h"
#include "em_rtc.h"

#include <le-rtc.h>

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
