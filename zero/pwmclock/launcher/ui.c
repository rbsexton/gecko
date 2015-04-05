/// @file ui.c
/// @brief A Rotary encoder UI

#include "ui.h"

// -----------------------------------------------------------
// Button State machine
// -----------------------------------------------------------
tUIButtonState UIState = tUIState_init;
int UIState_counter; 

void UIButtonUpdate(int pressed) {
	switch(UIState) {
		case tUIState_init:
			if (pressed) { UIState_counter = WAKE_COUNT; UIState = tUIState_waking; }
			break;
		case tUIState_waking:
			if (pressed) { 
				UIState_counter--;
				if ( UIState_counter <= 0 ) { UIState = tUIState_awake; }
				}
			else { UIState = tUIState_init; }
			break;
		case tUIState_awake:
			break;
		default:
			break;
	}
}

// -----------------------------------------------------------
// Animation State machine for the needles. 
// @returns 0 for off, 1 for on, -1 for normal
// -----------------------------------------------------------
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
