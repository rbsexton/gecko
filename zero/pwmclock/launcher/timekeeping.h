#define __TIMEKEEPING_H__

// Expects to be called at 16Hz.   Returns 1 if we have a new value.
typedef struct {
	int h, m, s;
	} sTimeHMS;

void timekeeping_init();

// Designed to be called at 16Hz
void TimeUpdate();
void DTimeUpdate();


int next_second();

// Target-specific things.
int next_second_pwm(int old);