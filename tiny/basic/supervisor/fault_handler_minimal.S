.text
.syntax unified

.thumb_func     @  Make sure the linker puts it in the table.
.global Fault_Handler
.global HardFault_Handler
.global MemManage_Handler
.global BusFault_Handler
.global UsageFault_Handler
.global DebugMon_Handler

@ The lightest possible fault handler.  It just fetches the correct
@ Stack pointer and loops.

HardFault_Handler:
MemManage_Handler:
BusFault_Handler:
UsageFault_Handler:
DebugMon_Handler:
Fault_Handler:

	tst	lr,#0x4		@ Figure out which stack
	ite	eq
	mrseq	r0,msp		@ Main stack
	mrsne	r0,psp		@ Process/Thread Stack
	mov r0, sp

loop:
	b loop 

