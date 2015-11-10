/// @file ui.c
/// @brief A Rotary encoder UI

/// Heres the basics of how this works.
/// Setting the Clock
/// 1. Hold the button for 3s.   The H and M needles go to zero.
/// Heres 
/// Attempt #2.  Build this so that it's somewhat like ladder logic.

#include <stdbool.h>

#include "ui.h"

void ui_init() {
	;
	}

// ------------------------------------------------------------
// State objects
// Here are the various counters that we can use as state inputs.
sCounter Input_pressed    = { 0, 2, false };
sCounter Input_pressed1s  = { 0, 16, false };
sCounter Input_pressed3s  = { 0, 48, false };
sCounter Input_pressed10s = { 0, 160, false };

sCounter Input_released   = { 0, 2, false };
sCounter Input_timeout    = { 0, 480, false }; // Tip after 30s

static void refresh_upcounter(sCounter *p, bool input) {
	if ( !input ) { p->value = 0; p->active = false; return; }
	p->value++;
	p->active = ( p->value >= p->limit ); 
	return;
	}

static void refresh_downcounter(sCounter *p, bool input) {
	refresh_upcounter(p, !input);
	}

void UpdateInputs(bool pressed) {
	refresh_downcounter(&Input_released, pressed);
	refresh_downcounter(&Input_timeout,  pressed);

	refresh_upcounter(&Input_pressed,    pressed);
	refresh_upcounter(&Input_pressed1s,  pressed);
	refresh_upcounter(&Input_pressed3s,  pressed);
	refresh_upcounter(&Input_pressed10s, pressed);
	}

// -----------------------------------------------------------
// Button State machine
// -----------------------------------------------------------
static tUIButtonState UIState = tUIState_run;

void UIStateUpdate() {	
	switch(UIState) {
		case tUIState_run:
			if (Input_timeout.active )  { UIState = tUIState_run; return; }
			if (Input_pressed3s.active) { UIState = tUIState_set_hours; return; }
		default:
			if (Input_timeout.active )  { UIState = tUIState_run; return; }
			break;
	}
}

// There is a presumption here that these get updated at 16Hz.
// Update the Needles.

void Needle0Update() {
	switch(UIState) {
		default:
			break;
		}
	
}

void NeedleUpdate() {
	Needle0Update();
	}

#if 0
tUINeedleState UINeedleState[3] = { 0,0,0 };
int UINeedle_counter[3]; 

int NeedleUpdate(int i) {
	switch(UINeedleState[i]) {
		case tNeedleState_run:
			if ( UIState == tUIState_waking ) {
				UINeedleState[i] = tNeedleState_bounce_h0;
				UINeedle_counter[i] = BOUNCE_COUNT;
				}
			return(-1);
			break;
		case tNeedleState_bounce_h0:
			if ( --UINeedle_counter[i] <= 0 ) { 
				UINeedleState[i] = tNeedleState_bounce_l0;
				UINeedle_counter[i] = BOUNCE_COUNT;
				}
			return(1);
			break;
		case tNeedleState_bounce_l0:
			if ( --UINeedle_counter[i] <= 0 ) { 
				UINeedleState[i] = tNeedleState_bounce_h1;
				UINeedle_counter[i] = BOUNCE_COUNT;
				}
			return(0);
			break;
		case tNeedleState_bounce_h1:
			if ( --UINeedle_counter[i] <= 0 ) { 
				UINeedleState[i] = tNeedleState_bounce_h1;
				UINeedle_counter[i] = BOUNCE_COUNT;
				}
			return(0);
			break;
		default: return(0);
	}	
}
#endif 