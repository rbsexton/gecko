// Interconnect between C and forth 
// Two parts -
//  Jump table of C functions that would be useful for calling from forth.
//  A small structure pinned to the very beginning of memory. 
// You could potentially use some reserved vectors, but not all Cortex-M
// devices but the reset vector in the same spot.  Thanks ST :-(

#include <stdint.h>

#include "interconnect.h"

tSharedData theshareddata __attribute__ ((section(".shareddata"))) = {
	0,-1,0
 	};

// Lie!
extern void EMU_EnterEM2();

void (* const jumptable[])(void) __attribute__ ((section(".jumptable"))) = {
	EMU_EnterEM2,
	};

