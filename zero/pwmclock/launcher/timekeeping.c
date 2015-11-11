#include <stdint.h>

#include "timekeeping.h"
#include "interconnect.h"
#include "bresenham.h"

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

// Expects to be called at 16Hz
static int count = 0;

void TimeUpdate() {
	sTimeHMS *p = theshareddata.tod_traditional;

	count++;
	if (count < 15 ) return; // If we haven't wrapped, return;
	count = 0;
	
	p->s++; if (p->s < 60 ) return; else p->s=0;
	p->m++; if (p->m < 60 ) return; else p->m=0;
	p->h++; if (p->h < 24 ) return; else p->h=0;
	
	return;
	}

// --------------------------------------------------------------
// --------------------------------------------------------------
// Decimal time.
// --------------------------------------------------------------
// --------------------------------------------------------------
// Hooks.
static void DTimeNewSecond() { ; }

// DTime requires an interpolator.

static tInterpKernel dither_dtime;
void DTimeUpdate() {
	sTimeHMS *p = theshareddata.tod_decimal;

	if ( interp_next(&dither_dtime) ) { // Only update on the decimal second.
		p->s++; if (p->s < 99 ) return; else p->s=0;
		p->m++; if (p->m < 99 ) return; else p->m=0;
		p->h++; if (p->h <  9 ) return; else p->h=0;
		DTimeNewSecond();
		}
	
	return;
	}

// --------------------------------------------------------------
// Initialization Hook
// --------------------------------------------------------------
void timekeeping_init() {
	theshareddata.tod_traditional = &TimeOfDay;
	theshareddata.tod_decimal = &TOD_Decimal;
	
	// Setup the interpolator.
	interp_init(&dither_dtime, 125, 1728); // Generate Decimal Seconds
	}



// Call this at 1Hz.
int next_second_pwm(int old) {
	return(old + 15);
	}
