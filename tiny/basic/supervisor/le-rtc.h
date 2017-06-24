#ifndef __SAPI_SYSCALLS_H__
#include "syscalls.h"
#endif

void setupRTC(void);

typedef enum {
	wakerequest_after0 = 0,
	wakerequest_every0 = 1,
	wakerequest_after1 = 2,
	wakerequest_every1 = 3,
	} tWakeRequestType;

bool le_rtc_callback_request(tWakeRequestType id, int arg, unsigned long *tcb);
