# Contents
- minimal-irqs - IRQ-Enabled Launcher plus tradtional forth.
- minimal-irqs-umb - IRQ-Enabled Launcher plus umbilical forth.

# System Control Notes

This code uses a 'launcher' to initialize the part prior to starting up forth.

Changes from the stock MPE Startup procedure (StartCortex.fth)
- Don't initialize the master stack pointer.  Let the launcher keep its' stack for exceptions.
- Disable interrupts before manipulating the user stack pointer
- Zero out the stack pointer at the start of the image.


