Base platform for non-umbilical, minimalist MPE forth on 
Silabs/Energy Micro Zero Gecko.

There is a minimal laucher, plus MPE Forth.

The Forth console is connected to  LEUART0.  The RTC runs at <>Hz, and
keeps time in ms, as per TICKS.  EM0 runs at <>Mhz, and the board mostly
lives in EM2

Power consumption is minimal: 100uA in idle, ~ 220uA in full performance.

Pinout:
LEUART0 TX: D4/J100 Pin 12/FTDI TTL-232R Yellow
LEUART0 RX: D5/J100 Pin 14/FTDI TTL-232R Orange.
Ground: J100 Pin 1

Board:  Silicon Labs EFM32ZG_STK3200 Starter Kit
Device: EFM32ZG222F32

