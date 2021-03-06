//
// Required system calls
// 
// Some of these are stubbed out.

#include <stdint.h>
#include <stdbool.h>
#include <ringbuffer.h>

#include "syscalls.h"

#include "console_leuart.h"
#include "le-rtc.h"

#include "interconnect.h"

void* __SAPI_01_GetRuntimeData(int i) {
	switch(i) {
		case 0: 
			return(0);
		case 1:
			return((void* )&jumptable);
		default:
			return(0);
		}
	}

extern RINGBUF rb_IN;

/// @parameters
/// @R0 - Stream Number
/// @R1 - The Character in question.
/// @returns in R0 - Result - 0 for success. 1 for blocked - Thread must yield/pause
bool __SAPI_02_PutChar(int stream, uint8_t c, unsigned long *tcb) {
	int ret;
	switch ( stream ) {
		default:
			return(console_leuart_putchar(stream, c, tcb));
		}
	}

// Support blocking by returning -1.
int __SAPI_03_GetChar(int stream, unsigned long *tcb) {
	switch ( stream ) {
		default:
			return(console_leuart_getchar(stream,tcb));
		}
	}

// Return the number of incoming characters.
int __SAPI_04_GetCharAvail(int stream) {
	switch ( stream ) {
		default:
			return(console_leuart_charsavailable(stream));
		}
	}

// Putstring - System call to support TYPE.
// Return 0 for success
bool __SAPI_05_PutString(int stream,  int len, uint8_t *p, unsigned long *tcb) {
	switch ( stream ) {
		default:
			return(0);
		}
	}

// Stubbed out.
bool __SAPI_06_EOL(int stream, unsigned long *tcb) {
	switch ( stream ) {
		default:
			return(false);
		}
	}

// Stubbed out
bool __SAPI_12_WakeRequest(int id, int arg, unsigned long *tcb) {
	return( le_rtc_callback_request( (unsigned) id, arg, tcb));
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
unsigned __SAPI_15_GetTime(tTimeType kind) {
	return(kind);
	// if (kind == 0) return(0);
	//else return( (unsigned) &rtc_ms );
}

