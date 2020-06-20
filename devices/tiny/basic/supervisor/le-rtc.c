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
  RTC_IntEnable(RTC_IF_COMP0|RTC_IF_COMP1);

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
	unsigned count;
	} sCompare;

sCompare callbackinfo[2];

bool le_rtc_callback_request(int id, int arg, unsigned long *tcb) {
	int ch = id & WAKEREQ_CH_MASK;
	uint32_t base;

	if ( id & WAKEREQ_RELATIVE_LAST ) base = RTC_CompareGet(ch); 
	else base = RTC_CounterGet();
	
	callbackinfo[ch].tcb = tcb;
	if ( arg == 0 ) return(false); // Check for disable.
	if (tcb) forth_thread_stop(tcb);
	RTC_CompareSet(ch, base+arg);
	return(true);
	}

void RTC_IRQHandler(void) {
    /* Don't clear them yet. It'll be cleaner if the IRQ */
	/* Happens again when there are multiple IRQs */
	uint32_t ints = RTC_IntGet();
	int ch;
	// RTC_IntClear(ints);
	if ( ints & RTC_IFC_COMP0 ) {
		RTC_IntClear(RTC_IFC_COMP0);
		ch = 0;
		}
	else if ( ints & RTC_IFC_COMP1 ) {
		RTC_IntClear(RTC_IFC_COMP1);
		ch = 1;
		}		
	else {
		RTC_IntClear(ints); // Just in case.
		return;
		}
	
	callbackinfo[ch].count++;

	if ( callbackinfo[ch].tcb ) {
		forth_thread_restart(callbackinfo[ch].tcb);
		callbackinfo[ch].tcb = 0;
		}
	}

