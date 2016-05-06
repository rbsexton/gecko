uint32_t USBGetChar(int stream,  unsigned long *tcb);
uint32_t USBGetCharAvail(int stream);
int USBPutChar(int usbstream, uint8_t c);
int USBOutEOL(int usbstream, unsigned long *tcb);

