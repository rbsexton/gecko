#ifndef __SAPI_SYSCALLS_H__
#include "syscalls.h"
#endif

void setupRTC(void);

// LSB is the channel.
#define WAKEREQ_CH0 0x0
#define WAKEREQ_CH1 0x1
#define WAKEREQ_CH_MASK 0x1

// Bit one is the type.
#define WAKEREQ_RELATIVE_NOW  0x0
#define WAKEREQ_RELATIVE_LAST 0x2

bool le_rtc_callback_request(int id, int arg, unsigned long *tcb);
