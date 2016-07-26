\ StartCortexThread.fth - generic Cortex startup code
\ Stripped down version of MPE version.
\ The supervisor does most of the important work.
\ so StartCortex is simpler.


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

\ Calculate the initial value for the data stack pointer.
\ We allow for TOS being in a register and guard space.

[undefined] sp-guard [if]		\ if no guard value is set
0 equ sp-guard
[then]

init-s0 tos-cached? sp-guard + cells -
  equ real-init-s0	\ -- addr
\ The data stack pointer set at start up.

internal
: StartCortex	\ -- ; never exits
\ *G Set up the Forth registers and start Forth. Other primary
\ ** hardware initialisation can also be performed here.
  begin
    \ INT_STACK_TOP SP_main sys! \ set SP_main for interrupts - we don't own it.
    \ INIT-R0 SP_process sys!	 \ set SP_process for tasks - the loader will do this.
    \ 2 control sys!			 \ switch to SP_process - handled by the loader. 
    REAL-INIT-S0 set-sp			 \ Allow for cached TOS and guard space
    INIT-U0 up!				\ USER area
    CLD1 @ execute
  again
;
external

\ -----------------------------------------------------
\ Define the Cortex-M vectors.  
\ For a client application, we only need two.
\ -----------------------------------------------------
INIT-R0 StackVec# SetExcVec	\ Define initial return stack
' StartCortex ResetVec# SetExcVec	\ Define startup word

\ ------------------------------------------
\ reset values for user and system variables
\ ------------------------------------------
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

\ ======
\ *> ###
\ ======

decimal

