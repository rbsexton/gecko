\ Mid-sized SAPI forth
\ This is a generic SAPI Binary to work with ABI 4.0


((
Change history
==============
))


\ ================
\ *! tiva
\ ================

only forth definitions  decimal


\ ******************************
\ Define the default directories
\ ******************************

\ MPE macros
c" ./Common" setmacro CommonDir	\ where the common code lives
c" ./Cortex" setmacro CpuDir		\ where the CPU specific code lives
c" ."		setmacro HWDir		\ where board specific code lives
c" ."    	setmacro AppDir		\ where application code lives

c" ./Drivers" setmacro DriverDir \ Driver Code

c" ../../../cm3forthtools/"    setmacro LocalCM3	\ Cortex-M common forth 
c" ../../../sockpuppet/forth"    setmacro SAPIDir	\  Low-Level SAPI stuff.


\ ***************************************
\ Load compiler extensions such as macros
\ ***************************************

include %CpuDir%/Macros

\ *********************************************************
\ Turn on the cross compiler and define CPU and log options
\ *********************************************************

\ file: zero.log                \ uncomment to send log to a file

CROSS-COMPILE

only forth definitions          \ default search order

  Cortex-M3                     \ Thumb-2 processor type and register usage

  \ no-log                        \ uncomment to suppress output log
  rommed                        \ split ROM/RAM target
  interactive                   \ enter interactive mode at end
Stamp? 0= [if] +xrefs [then]	\ enable cross references
  align-long                    \ code is 32bit aligned
  +LongCalls			\ permit standalone Forth to handle
  				\ calls outside 25 bit range.
 hex-i32   

0 equ false
-1 equ true

\ *******************
\ *S Configure target
\ *******************

\ =====================
\ *N Low Memory Definitions.
\ =====================

#16 cells equ /ExcVecs	\ -- len
\ *G Size of the exception/interrupt vector table. There are
\ ** This isn't really used in a SAPI
\ ** 16 reserved by ARM + the 17 for the Zero Gecko

\ =============
\ *N Memory map
\ =============


\ *P The Flash memory starts at $0000:0000. 
\ This is a 256/32k part.  We get the top half of flash, plus
\ 3/4 of the RAM.

  $0002:0000 $0002:FFFF cdata section Leopard	\ code section in boot Flash
  $2000:2000 $2000:6fff idata section PROGd	\  IDATA - Interactive code here.
  $2000:7000 $2000:7FFF udata section PROGu	\  UDATA

interpreter
: prog Leopard ;			\ synonym
target

PROG PROGd PROGu  CDATA                 \ use Code for HERE , and so on


\ ============================
\ *N Stack and user area sizes
\ ============================

$100 equ UP-SIZE		\ size of each task's user area
$080 equ SP-SIZE		\ size of each task's data stack
$100 equ RP-SIZE		\ size of each task's return stack
up-size rp-size + sp-size +
  equ task-size			\ size of TASK data area
\ define the number of cells of guard space at the top of the data stack
#4 equ sp-guard			\ can underflow the data stack by this amount

$080 equ TIB-LEN		\ terminal i/p buffer length

\ define nesting levels for interrupts and SWIs.
0 equ #IRQs			\ number of IRQ stacks,
				\ shared by all IRQs (1 min)
0 equ #SVCs			\ number of SVC nestings permitted
				\ 0 is ok if SVCs are unused

#10 equ console-port	\ -- n ; Designate serial port for terminal (0..n).
  0 equ useUART0? 

\ *G Ports 1..5 are the on-chip UARTs. The internal USB device
\ ** is port 10, and bit-banged ports are defined from 20 onwards.

#10 equ tick-ms		\ -- ms
\ *G Timebase tick in ms.

\ ***************************************
\ Load compiler extensions such as macros
\ ***************************************


\ =====================
\ *N Software selection
\ =====================

\ Kernel components
 1 equ ColdChain?		\ nz to use cold chain mechanism
 1 equ tasking?			\ true if multitasker needed
   #6 cells equ tcb-size	\   for internal consistency check
   1 equ irqsafe-usermode?  \ Use LDREX/STREX for interrupt safety.
   1 equ event-handler?		\   true to include event handler
   0 equ message-handler?	\   true to include message handler
   0 equ semaphores?		\ true to include semaphores
 0 equ timebase?		\ true for TIMEBASE code
 0 equ softfp?			\ true for software floating point
 0 equ FullCase?		\ true to include ?OF END-CASE NEXTCASE extensions
 0 equ target-locals?		\ true if target local variable sources needed
 0 equ romforth?		\ true for ROMForth handler
 0 equ blocks?			\ true if BLOCK needed
 $0000 equ sizeofheap		\ 0=no heap, nz=size of heap
   1 equ heap-diags?		\   true to include diagnostic code
 0 equ paged?			\ true if ROM or RAM is paged/banked
 0 equ ENVIRONMENT?		\ true if ANS ENVIRONMENT system required

\ *****************
\ default constants
\ *****************

cell equ cell				\ size of a cell (16 bits)
0 equ false
-1 equ true

 0 equ InitializeMSP? \ Don't initialize system stacks.
 1 equ SAPIWakeSupport? \ System calls wrappers should pass UP.

\ ************
\ *S Kernel files
\ ************
  include %CpuDir%/CM3Def		\ Cortex generic equates and SFRs
  include %CpuDir%/StackDef		\ Reserve default task and stacks
  PROGd  sec-top 1+ equ UNUSED-TOP  PROG	\ top of memory for UNUSED

  include %AppDir%/StartCortexThread		\ Thread-mode Cortex-M3 startup code
  include %CpuDir%/CodeCortex		\ low level kernel definitions

  include %CommonDir%/kernel62          \ high level kernel definitions 

  include %CommonDir%/Devtools		\ DUMP .S etc development tools
  include %CommonDir%/DebugTools	
  include %CommonDir%/Voctools		\ ORDER VOCS etc
  include %CommonDir%/methods		\ target support for methods
  \ include %CpuDir%/LocalCM3		\ local variables


 
  include %SAPIDir%/SysCalls		\ System Calls.  Define these early.
  \ include %SPDir%/dylink     		\ Runtime Linking

  include %CpuDir%/MultiCortex		\ multi-tasker, MUST be before TIMEBASE
  include %LocalCM3%/pause     		\ A customized version of pause.
  include %LocalCM3%/idlewfi     	\ A task that can call wfi.  Low-budget power savings.
  \ include %AppDir%/LeopardGecko		\ Various Addresses


timebase? [if]
  include %CommonDir%/timebase		\ time base common code, MUST be before SysTickxxx
  include %CommonDir%/Delays		\ time delays
  ' start-timers AtCold
[else]
  include %CommonDir%/Delays
[then]

environment? [if]
  include %CommonDir%/environ		\ ENVIRONMENT?
[then]

SIZEOFHEAP [if]
  include %CommonDir%/heap32		\ memory allocation set
[then]

\ *************
\ *S End of kernel
\ *************

buildfile leopard.no
l: version$
  build$,
l: BuildDate$
  DateTime$,

internal
: .banner	\ --
  cr ." ********************************"
;

: .CPU		\ -- ; display CPU type
  ." MPE ROM PowerForth for Cortex-M3" cr
  version$ $. space BuildDate$ $.
;
external

: ANS-FORTH	\ -- ; marker
;


\ *******************
\ *S Application code
\ *******************

include %SAPIDir%/serCM3_SAPI-level0 \ polled serial driver

\ include %AppDir%/startup
' hex AtCold

\ ***************
\ *S Finishing up
\ ***************

libraries	\ to resolve common forward references
  include %CpuDir%/LibCortex
  include %CommonDir%/LIBRARY
end-libs

\ Force binary file to 512 byte unit.
cdata
  flush-idata                           \ force IDATA sections to be laid
  					\ NOW, rather than by FINIS
  here $1FF +  $-0200 and org		\ force to 512 byte boundary for ISP loader

decimal

\ XREF DUP                              \ where is DUP used?
\ XREF-ALL                              \ full cross reference
\ XREF-UNUSED                           \ find unused words

update-build
FINIS

\ ======
\ *> ###
\ ======

