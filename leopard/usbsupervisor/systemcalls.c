//
// Required system calls
// 
// Some of these are stubbed out.

#include <stdint.h>
#include <stdbool.h>
#include <ringbuffer.h>

#include "usb-sapi.h"

void __SAPI_01_GetRuntimeData(int i) {
	return(0);
}

extern RINGBUF rb;

/// @parameters
/// @R0 - Stream Number
/// @R1 - The Character in question.
/// @returns in R0 - Result - 0 for success. -1 for highwater, -2 for fail
int32_t __SAPI_02_PutChar(unsigned long stream, uint8_t c) {
	int ret;
	switch ( stream ) {
		case 10:
		case 11:
			// return(USBPutChar(stream-1, c));
			break;
		default:
			ret = ringbuffer_addchar(&rb,c);
		}
	return(ret);
	}

uint32_t __SAPI_03_GetChar(unsigned long stream) {
	switch ( stream ) {
		case 10:
		case 11:
			// return(USBGetChar(stream-1));
			break;
		default:
			return(0);
		}
	}

uint32_t __SAPI_04_GetCharAvail(unsigned long stream) {
	switch ( stream ) {
		case 10:
		case 11:
			// return(USBGetCharAvail(stream-1));
			break;
		default:
			return(0);
		}
	}

uint32_t __SAPI_05_PutString(unsigned long stream, uint8_t *c, int len) {
	return(len);
}

uint32_t __SAPI_06_EOL(unsigned long stream) {
	return(0);
}

uint32_t __SAPI_14_PetWatchdog(unsigned long machineticks) {
	return(0);
}

uint32_t __SAPI_15_GetTimeMS(bool thirtytwo) {
	if (thirtytwo) 	return(0);
	else return(1);
}

