uint32_t USBGetChar(int stream,  unsigned long *tcb);
uint32_t USBGetCharAvail(int stream);
int USBPutChar(int usbstream, uint8_t c);
int USBPutString(int usbstream, int len, uint8_t *p,  unsigned long *tcb);
int USBOutEOL(int usbstream, unsigned long *tcb);
