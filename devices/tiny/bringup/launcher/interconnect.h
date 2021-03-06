// Interconnect between C and forth 
// Two parts -
//  Jump table of C functions that would be useful for calling from forth.
//  A small structure pinned to the very beginning of memory. 
// You could potentially use some reserved vectors, but not all Cortex-M
// devices but the reset vector in the same spot.  Thanks ST :-(

#include <stdint.h>

typedef struct {
	uint32_t *jumptable;
	uint32_t u0rxdata;
} tSharedData;

extern tSharedData theshareddata;

