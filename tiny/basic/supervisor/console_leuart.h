void initLeuart(void);
void console_leuart_init(); 

// Interfaces for use by system calls.
bool console_leuart_putchar(int stream, int c, unsigned long *tcb);
int  console_leuart_charsavailable(int stream);
int console_leuart_getchar(int stream, unsigned long *tcb);
