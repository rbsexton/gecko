// Interconnect between C and forth 
// Two parts -
//  Jump table of C functions that would be useful for calling from forth.
//  A small structure pinned to the very beginning of memory. 
// You could potentially use some reserved vectors, but not all Cortex-M
// devices put the reset vector in the same spot.  Thanks ST :-(

// Don't work too hard to put these things into Flash.  SRAM will be faster
// on some devices.

#include <stdint.h>

#include "interconnect.h"

// Lie!
extern void EMU_EnterEM2();

extern void SegmentLCD_Write();
extern void SegmentLCD_AllOn(); 
extern void SegmentLCD_AllOff();
extern void SegmentLCD_AlphaNumberOff();
extern void SegmentLCD_Number();
extern void SegmentLCD_NumberOff();
extern void SegmentLCD_ARing();
extern void SegmentLCD_Battery(); 
extern void SegmentLCD_Symbol();

void (* const jumptable[])(void) = {
	SegmentLCD_Write,
//	SegmentLCD_AlphaNumberOff,
//	SegmentLCD_Number,
//	SegmentLCD_NumberOff,
//	SegmentLCD_Symbol,
//	SegmentLCD_AllOff,
//	SegmentLCD_AllOn, 
//	SegmentLCD_ARing,
//	SegmentLCD_Battery, 
	};


tSharedData theshareddata = {
		(uint32_t *) &jumptable,
		0xffffffff
	 	};

