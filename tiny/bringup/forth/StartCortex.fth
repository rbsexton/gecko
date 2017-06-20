\ StartCortex.fth - generic Cortex startup code

((
Copyright (c) 2009, 2010
MicroProcessor Engineering
133 Hill Lane
Southampton SO15 5AF
England

tel:   +44 (0)23 8063 1441
fax:   +44 (0)23 8033 9691
email: mpe@mpeforth.com
       tech-support@mpeforth.com
web:   http://www.mpeforth.com
Skype: mpe_sfp

From North America, our telephone and fax numbers are:
       011 44 23 8063 1441
       011 44 23 8033 9691
       901 313 4312 (North American access number to UK office)


To do
=====

Change history
==============
20100129 MPE002 Updated to compiler build 430.
20091223 MPE001 First release.
))

only forth definitions  decimal

\ ===========
\ *! initcortex
\ *T Generic Cortex start up
\ ===========

l: ExcVecs	\ -- addr ; start of vector table
\ *G The exception vector table is *\fo{/ExcVecs} bytes long. The
\ ** equate is defined in the control file.
  /ExcVecs allot&erase

interpreter
: SetExcVec	\ addr exc# --
\ *G Set the given exception vector number to the given address.
\ ** Note that for vectors other than 0, the Thumb bit is forced
\ ** to 1.
  dup if				\ If not the stack top
    swap 1 or swap			\   set the Thumb bit
  endif
  cells ExcVecs + !  ;
target

L: CLD1		\ holds xt of main word
  0 ,					\ fixed by MAKE-TURNKEY

proc DefExc2	b  $	end-code	\ NMI
proc DefExc3	b  $	end-code	\ HardFault
proc DefExc4	b  $	end-code	\ MemManage
proc DefExc5	b  $	end-code	\ BusFault
proc DefExc6	b  $	end-code	\ UsageFault
proc DefExc11	b  $	end-code	\ SVC
proc DefExc12	b  $	end-code	\ DbgMon
proc DefExc14	b  $	end-code	\ PendSV
proc DefExc15	b  $	end-code	\ SysTick


\ Calculate the initial value for the data stack pointer.
\ We allow for TOS being in a register and guard space.

[undefined] sp-guard [if]		\ if no guard value is set
0 equ sp-guard
[then]

init-s0 tos-cached? sp-guard + cells -
  equ real-init-s0	\ -- addr
\ The data stack pointer set at start up.


udata \ This has got to be part of udata
create icroot 4 allot \ Values are cleaner, but they're part of IDATA..
cdata

internal

\ Sneakery to get r0.
code get_icroot
	str tos, [ psp, # -4 ] !
    mov r7, r0	
	next, 
end-code 

: StartCortex	\ -- ; never exits
\ *G Set up the Forth registers and start Forth. Other primary
\ ** hardware initialisation can also be performed here.
  begin
    \ INT_STACK_TOP SP_main sys!		\ set SP_main for interrupts
    INIT-R0 SP_process sys!		\ set SP_process for tasks
    2 control sys!			\ switch to SP_process
    REAL-INIT-S0 set-sp			\ Allow for cached TOS and guard space

    get_icroot \ Do this before anything else tampers with R0.
    icroot ! 

    INIT-U0 up!				\ USER area
    CLD1 @ execute
  again
;
external

INIT-R0 StackVec# SetExcVec	\ Define initial return stack
' StartCortex ResetVec# SetExcVec	\ Define startup word

\ ------------------------------------------
\ reset values for user and system variables
\ ------------------------------------------

[undefined] umbilical? [if]
\  true
\ [else]
\   umbilical? 0=
\ [then]
\ [if]
\ initial values of user variables
L: USER-RESET
  init-s0 tos-cached? sp-guard + cells - ,	\ s0
  init-r0 ,				\ r0
  0 ,  0 ,                              \ #tib, 'tib
  0 ,  0 ,                              \ >in, out
  $0A ,  0 ,                            \ base, hld

\ initial values of system variables
L: INIT-FENCE 0 ,                       \ fence
L: INIT-DP 0 ,                          \ dp
L: INIT-VOC-LINK 0 ,                    \ voc-link
[then]


\ ======
\ *> ###
\ ======

decimal

