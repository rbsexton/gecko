//
// Required system calls
// 
// Some of these are stubbed out.

#include <stdint.h>
#include <stdbool.h>
#include <ringbuffer.h>

#include "syscalls.h"
#include "usb-sapi.h"

#include "console_leuart.h"

long* __SAPI_01_GetRuntimeData(int i) {
	if ( i == 0 ) return(0);
	else return(0);
}

extern RINGBUF rb_IN;

/// @parameters
/// @R0 - Stream Number
/// @R1 - The Character in question.
/// @returns in R0 - Result - 0 for success. 1 for blocked - Thread must yield/pause
bool __SAPI_02_PutChar(int stream, uint8_t c, unsigned long *tcb) {
	int ret;
	switch ( stream ) {
		case 0: 
			return(console_leuart_putchar(c, tcb));
		case 10:
		case 11:
			return(USBPutChar(stream-10, c));
			break;
		default:
			ret = ringbuffer_addchar(&rb_IN,c);
		}
	return(ret);
	}

// Support blocking by returning -1.
int __SAPI_03_GetChar(int stream, unsigned long *tcb) {
	switch ( stream ) {
		case 0:
			return(console_leuart_getchar(tcb));
		case 10:
		case 11:
			return(USBGetChar(stream-10, tcb));			
			break;
		default:
			return(-1);
		}
	}

// Return the number of incoming characters.
int __SAPI_04_GetCharAvail(int stream) {
	switch ( stream ) {
		case 0:
			return(console_leuart_charsavailable());
		case 10:
		case 11:
			return(USBGetCharAvail(stream-10));
			break;
		default:
			return(0);
		}
	}

// Putstring - System call to support TYPE.
// Return 0 for success
bool __SAPI_05_PutString(int stream,  int len, uint8_t *p, unsigned long *tcb) {
	switch ( stream ) {
		case 10:
		case 11:
			return(USBPutString(stream-10,len,p,tcb));
			break;
		default:
			return(0);
		}
	}

// Stubbed out.
bool __SAPI_06_EOL(int stream, unsigned long *tcb) {
	switch ( stream ) {
		case  0:
			return(USBOutEOL(stream-10,tcb));
		case 10:
		case 11:
			return(USBOutEOL(stream-10,tcb));
			break;
		default:
			return(0);
		}
	}

// Stubbed out
unsigned __SAPI_12_SetWakeRequest(int id, unsigned *addr, unsigned mask) {
	if ( id == 0 ) return(*addr);
	else return(mask);
	}

// Stubbed out
unsigned __SAPI_13_CPUUsage() {
	static unsigned count = 0;
	return(++count);
	}

// Stubbed out
void __SAPI_14_PetWatchdog(unsigned machineticks) {
	static unsigned count = 0;
	count += machineticks;
	return;
}

extern uint32_t rtc_ms;
unsigned __SAPI_15_GetTimeMS(int kind) {
	if (kind == 0) return(rtc_ms);
	else return( (unsigned) &rtc_ms );
}

