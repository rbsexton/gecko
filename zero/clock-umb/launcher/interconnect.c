// Interconnect between C and forth 
// Two parts -
//  Jump table of C functions that would be useful for calling from forth.
//  A small structure pinned to the very beginning of memory. 
// You could potentially use some reserved vectors, but not all Cortex-M
// devices but the reset vector in the same spot.  Thanks ST :-(

#include <stdint.h>

#include "em_chip.h"
#include "em_device.h"
#include "em_msc.h"

#include "interconnect.h"

tSharedData theshareddata __attribute__ ((section(".shareddata"))) = 
 	{ 0,-1,0};

// Lie!
extern void EMU_EnterEM2();

int addone(int i) { return(i+1); }
int addtwo(int i, int j) { return(i+j); } 
int addthree(int i, int j, int k ) { return(i+j+k); } 

void (* const jumptable[])(void) __attribute__ ((section(".jumptable"))) = {
	EMU_EnterEM2,
	MSC_ErasePage,
	MSC_WriteWord,
	addone,
	addtwo,
	addthree
	};

void InitSharedData() {
	theshareddata.jumptable = jumptable;
}

