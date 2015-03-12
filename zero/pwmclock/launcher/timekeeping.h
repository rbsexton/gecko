// Expects to be called at 16Hz.   Returns 1 if we have a new value.
int next_second();

// Target-specific things.
int next_second_pwm(int old);