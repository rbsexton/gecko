void initLeuart(void);
void console_leuart_init(); 

// Interfaces for use by system calls.
bool console_leuart_putchar(int c, unsigned long *tcb);
int  console_leuart_charsavailable();
int console_leuart_getchar(unsigned long *tcb);
