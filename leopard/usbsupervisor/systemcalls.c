//
// Required system calls
// 
// Some of these are stubbed out.

#include <stdint.h>
#include <stdbool.h>
#include <ringbuffer.h>

#include "syscalls.h"
#include "usb-sapi.h"


long* __SAPI_01_GetRuntimeData(int i) {
	if ( i == 0 ) return(0);
	else return(0);
}

extern RINGBUF rb_IN;

/// @parameters
/// @R0 - Stream Number
/// @R1 - The Character in question.
/// @returns in R0 - Result - 0 for success. -1 for highwater, -2 for fail
int __SAPI_02_PutChar(int stream, uint8_t c, unsigned long *tcb) {
	int ret;
	switch ( stream ) {
		case 10:
		case 11:
			return(USBPutChar(stream-10, c));
			break;
		default:
			ret = ringbuffer_addchar(&rb_IN,c);
		}
	return(ret);
	}

int __SAPI_03_GetChar(int stream, unsigned long *tcb) {
	switch ( stream ) {
		case 10:
		case 11:
			return(USBGetChar(stream-10, tcb));			
			break;
		default:
			return(-1);
		}
	}

int __SAPI_04_GetCharAvail(int stream) {
	switch ( stream ) {
		case 10:
		case 11:
			return(USBGetCharAvail(stream-10));
			break;
		default:
			return(0);
		}
	}

int __SAPI_05_PutString(int stream, uint8_t *c, int len, unsigned long *tcb) {
	return(len);
}

// Stubbed out.
int __SAPI_06_EOL(int stream) {
	return(stream);
}

// Stubbed out
unsigned __SAPI_13_CPUUsage() {
	static unsigned count = 0;
	return(++count);
	}

// Stubbed out
void __SAPI_14_PetWatchdog(unsigned machineticks) {
	return;
}

unsigned __SAPI_15_GetTimeMS(int kind) {
	if (kind) 	return(0);
	else return(1);
}

