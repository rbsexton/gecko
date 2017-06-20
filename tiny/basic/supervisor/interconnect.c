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
extern int main();

void (* const jumptable[])(void) = {
	EMU_EnterEM2,
	};


tSharedData theshareddata = {
		(uint32_t *) &jumptable,
		0xffffffff
	 	};

