#ifndef __SAPI_SYSCALLS_H__
#include "syscalls.h"
#endif

void setupRTC(void);
bool le_rtc_callback_request(tWakeRequestType id, int arg, unsigned long *tcb);
