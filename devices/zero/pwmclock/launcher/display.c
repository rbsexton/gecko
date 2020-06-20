/// @file display.c

/// Things related to the updating of needles.   Keep in mind that 
/// they have to be calibrated.

#include "bresenham.h"

#include "display.h"

#define CC0_MAX 887 
#define CC1_MAX 870 
#define CC2_MAX 890 
	
static sNeedleVal NeedleMaxPWM   = { CC0_MAX, CC1_MAX, CC2_MAX};
sNeedleVal NeedleValHMS   = { 0,0,0 };
sNeedleVal NeedleValDTime = { 0,0,0 };

// The interpolators
tInterpKernel dither_hmS;
tInterpKernel dither_hMs;
tInterpKernel dither_Hms;

tInterpKernel dither_dtime_hmS;
tInterpKernel dither_dtime_hMs;
tInterpKernel dither_dtime_Hms;

// Functions for updating the state of the display information,
// Called once per second.

// -------------------------------
// Helpers
// -------------------------------
static void display_advance_s(sNeedleVal *current, tInterpKernel *kern) {
	current->s += interp_next(kern);
	if ( current->s > NeedleMaxPWM.s ) {
		interp_reset(kern);
		current->s = 0;
		}
	}		
static void display_advance_m(sNeedleVal *current, tInterpKernel *kern) {
	current->m += interp_next(kern);
	if ( current->m > NeedleMaxPWM.m ) {
		interp_reset(kern);
		current->m = 0;
		}
	}
static void display_advance_h(sNeedleVal *current, tInterpKernel *kern) {
	current->h += interp_next(kern);
	if ( current->h > NeedleMaxPWM.h ) {
		interp_reset(kern);
		current->h = 0;
		}
	}		

void DisplayAdvanceDTime() {
	display_advance_s(&NeedleValDTime, &dither_dtime_hmS);
	display_advance_m(&NeedleValDTime, &dither_dtime_hMs);
	display_advance_h(&NeedleValDTime, &dither_dtime_Hms);
	}

void DisplayAdvanceSecond() {
	display_advance_s(&NeedleValHMS, &dither_hmS);
	display_advance_m(&NeedleValHMS, &dither_hMs);
	display_advance_h(&NeedleValHMS, &dither_Hms);
	}

void display_init() {
	interp_init(&dither_hmS, NeedleMaxPWM.s,           60 );
	interp_init(&dither_hMs, NeedleMaxPWM.m,      60 * 60 );
	interp_init(&dither_Hms, NeedleMaxPWM.h, 12 * 60 * 60 );

	interp_init(&dither_dtime_hmS, NeedleMaxPWM.s,    100);
	interp_init(&dither_dtime_hMs, NeedleMaxPWM.m,  10000);
	interp_init(&dither_dtime_Hms, NeedleMaxPWM.h, 100000);
	}

