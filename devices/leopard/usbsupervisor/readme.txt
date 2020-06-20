This is a hacked-up version of the SiLabs composite device demo.

Its been converted into a sockpuppet service layer for forth.

Key bits that are officially part of the sockpuppet/sapi code base:

pendsv-launcher.c - This startups up the forth environment.
svchandler.S - The system call handler.  The system calls themselves
 are part of the usbsupervisor.

This was prototyped on this board:
Board:  Silicon Labs EFM32LG-STK3600 Development Kit
Device: EFM32LG990F256
