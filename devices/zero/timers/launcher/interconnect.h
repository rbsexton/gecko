// Interconnect between C and forth 
// Two parts -
//  Jump table of C functions that would be useful for calling from forth.
//  A small structure pinned to the very beginning of memory. 
// You could potentially use some reserved vectors, but not all Cortex-M
// devices but the reset vector in the same spot.  Thanks ST :-(

#include <stdint.h>

typedef struct {
	uint32_t jumptable;
	uint32_t u0rxdata;
	
	uint32_t count_a_sec;
	uint32_t count_a_min;
	uint32_t count_a_h;

	uint32_t count_b_sec;
	uint32_t count_b_min;
	uint32_t count_b_h;

	uint32_t count_c_sec;
	uint32_t count_c_min;
	uint32_t count_c_h;
	
} tSharedData;

extern tSharedData theshareddata;
extern void (* const jumptable[])(void);
void InitSharedData();
