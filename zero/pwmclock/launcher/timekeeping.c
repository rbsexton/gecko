#include "timekeeping.h"
#include "interconnect.h"

// Expects to be called at 16Hz.
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
