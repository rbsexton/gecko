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


// Expects to be called at 16Hz. - Not currently used.
int next_second() {
	static int count = 0;
	tSharedData *p = &theshareddata;
	count++;
	if (count < 16 ) return(0); // If we haven't wrapped, return 0;
	
	count = 0;
	
	p->count_a_sec++;
	if (p->count_a_sec < 60 ) return(1);
	p->count_a_sec=0;
	
	p->count_a_min++;
	if (p->count_a_min < 60 ) return(1);
	p->count_a_min=0;

	p->count_a_h++;
	if (p->count_a_h < 24 ) return(1);
	p->count_a_h=0;
	
	return(1);
	}

// Call this at 1Hz.
int next_second_pwm(int old) {
	return(old + 15);
	}
