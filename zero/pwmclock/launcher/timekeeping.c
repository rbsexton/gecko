#include <stdint.h>

#include "timekeeping.h"
#include "interconnect.h"

// Notes on timekeeping.    
// There are two timebases, traditonal and decimal.
// The fundamental timebase of the system is a 16Hz low-power oscillator.
// For time in seconds, things are simple - just increment the seconds 
// 1/16th of the time. 

// ------------------------------------------------------
// Data Structures
// ------------------------------------------------------
sTimeHMS TimeOfDay;
sTimeHMS TOD_Decimal;
	
//
void timekeeping_init() {
	theshareddata.tod_traditional = &TimeOfDay;
	theshareddata.tod_decimal = &TOD_Decimal;
	}

// Expects to be called at 16Hz.
void TimeUpdate() {
	static int count = 0;
	sTimeHMS *p = theshareddata.tod_traditional;

	count++;
	if (count < 16 ) return; // If we haven't wrapped, return;
	count = 0;
	
	p->s++; if (p->s < 60 ) return; else p->s=0;
	p->m++; if (p->m < 60 ) return; else p->m=0;
	p->h++; if (p->h < 24 ) return; else p->h=0;
	
	return;
	}

// Call this at 1Hz.
int next_second_pwm(int old) {
	return(old + 15);
	}
