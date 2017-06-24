#include <stdio.h>
#include "em_device.h"
#include "em_rtc.h"

#include "le-rtc.h"

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
  RTC_IntEnable(RTC_IF_COMP0);

  /* Enable RTC interrupt vector in NVIC */
  NVIC_EnableIRQ(RTC_IRQn);

  /* Enable RTC */
  RTC_Enable(true);
}

static inline void forth_thread_stop(uint32_t *tcb) {			
	tcb[2] &= ~1; // Clear it using the unsafe technique.				
	}

static inline void forth_thread_restart(uint32_t *tcb) {			
	tcb[2] |= 1;
	}


// Define one struct per Comparator.

typedef struct {
	unsigned long *tcb;
	int after;
	int every;
	} sCompare;

sCompare callbackinfo[2];


bool le_rtc_callback_request(tWakeRequestType id, int arg, unsigned long *tcb) {
	uint32_t now = RTC_CounterGet();
	uint32_t then;
	switch(id) {
		case wakerequest_after0:
			then = now + arg;
			callbackinfo[0].tcb = tcb;
			callbackinfo[0].after = arg;
			if ( arg == 0 ) return(false); // Check for disable.
			if (tcb) forth_thread_stop(tcb);
			RTC_CompareSet(0, then);
			return(true);
		case wakerequest_every0:
			then = now + arg;
			callbackinfo[0].tcb = tcb;
			callbackinfo[0].every = arg;
			if ( arg == 0 ) return(false); // Check for disable.
			if (tcb) forth_thread_stop(tcb);
			RTC_CompareSet(0, then);
			return(true);
		default:
			return(false);
		}
	}

void RTC_IRQHandler(void) {
  /* Clear interrupt source */
	uint32_t ints = RTC_IntGet();
	RTC_IntClear(ints);
	if ( (ints & RTC_IFC_COMP0) && callbackinfo[0].tcb ) {
		uint32_t *tcb = callbackinfo[0].tcb;
		if (callbackinfo[0].after) callbackinfo[0].after = 0;
		if (callbackinfo[0].every) {
			uint32_t now  = RTC_CounterGet();
			uint32_t then = now + callbackinfo[0].every;
			RTC_CompareSet(0, then);
			}		
		if (tcb) forth_thread_restart(tcb);
		}
	}

