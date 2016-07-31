\ zerobringup
\ Minimalist forth for brining up the zero gecko.


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

c" ../../../../cm3forthtools/"    setmacro LocalCM3	\ Cortex-M common forth 
c" ../../ZeroLib/" setmacro ZeroLib \ Code for the Zero Gecko


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

  Cortex-M0                     \ Thumb-2 processor type and register usage

  no-heads                      \ heads not needed for Umbilical Forth

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
\ *N Zero Gecko Definitions.
\ =====================

#16 #17 + cells equ /ExcVecs	\ -- len
\ *G Size of the exception/interrupt vector table. There are
\ ** 16 reserved by ARM + the 17 for the Zero Gecko

\ =============
\ *N Memory map
\ =============


\ *P The Flash memory starts at $0000:0000. 
\ We own the whole thing, but we have to start at a 1k boundary.
\ to leave room for the launcher

  \ Define the rest of the memory so that we can examine it.
  \ $0000:0000 $0000:2FFF udata section superc	\ Supervisor Code
  \ $0FE0:0000 $0FE0:03FF udata section userd     \ User Data
  \ $2000:0000 $2000:03FF udata section superr	\ Supervisor RAM.
  $0000:0000 $FFFF:FFFF udata section catchall	\ Everything else, incl peripherals.

  $0000:3000 $0000:FFFF cdata section Zero	\ code section in boot Flash
  $2000:0600 $2000:09FF idata section PROGd	\  IDATA - Dictionary is here.
  $2000:0A00 $2000:0FFF udata section PROGu	\  UDATA - Stacks are here.

interpreter
: prog Zero ;			\ synonym
target

PROG PROGd PROGu  CDATA                 \ use Code for HERE , and so on


\ ============================
\ *N Stack and user area sizes
\ ============================

$100 equ UP-SIZE		\ size of each task's user area
$080 equ SP-SIZE		\ size of each task's data stack
$080 equ RP-SIZE		\ size of each task's return stack
up-size rp-size + sp-size +
  equ task-size			\ size of TASK data area
\ define the number of cells of guard space at the top of the data stack
#4 equ sp-guard			\ can underflow the data stack by this amount

$30 equ max-depth               \ max depth for UF data stack before UF barfs

$080 equ TIB-LEN		\ terminal i/p buffer length

\ define nesting levels for interrupts and SWIs.
0 equ #IRQs			\ number of IRQ stacks,
				\ shared by all IRQs (1 min)
0 equ #SVCs			\ number of SVC nestings permitted
				\ 0 is ok if SVCs are unused

#0 equ console-port	\ -- n ; Designate serial port for terminal (0..n).
 1 equ useUART0? 
\ *G Ports 1..5 are the on-chip UARTs. The internal USB device
\ ** is port 10, and bit-banged ports are defined from 20 onwards.
#9600 equ console0-speed      \ Serial port speed in BPS.


#10 equ tick-ms		\ -- ms
\ *G Timebase tick in ms.

\ ***************************************
\ Load compiler extensions such as macros
\ ***************************************


\ =====================
\ *N Software selection
\ =====================

\ Kernel components

 1 equ umbilical?             \ true = Umbilical Forth, zero = standalone
 0 umbilical? and equ SPI?    \ true for SPI Umbilical, false for serial
  SPI? 0= equ Serial?         \ true for Serial Umbilical, false for SPI
 1 equ init-idata?              \ true if IDATA to be initialised

 1 equ ColdChain?		\ nz to use cold chain mechanism
 0 equ tasking?			\ true if multitasker needed
   #16 cells equ tcb-size		\   for internal consistency check
   0 equ event-handler?		\   true to include event handler
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

interpreter
: AtCold        \ xt --
\ *G Most Umbilical Forth systems do not use the *\fo{ColdChain}
\ ** mechanism provided in *\i{Kernel62.fth} for standalone
\ ** Forths. To prevent compilation problems, define *\fo{AtCold}
\ ** as *\fo{DROP}. Compilers built from December 2008 onwards
\ ** will warn you if *\fo{AtCold} is used before being defined
\ ** in the target code.
 drop
 ;

target

\ *****************
\ default constants
\ *****************

cell equ cell				\ size of a cell (16 bits)
0 equ false
-1 equ true


\ ************
\ *S Kernel files
\ ************
  include %CpuDir%/CM0Def		\ Cortex generic equates and SFRs
  include %CpuDir%/StackDef		\ Reserve default task and stacks
  PROGd  sec-top 1+ equ UNUSED-TOP  PROG	\ top of memory for UNUSED
  
  include %AppDir%/StartCM0		\ basic Cortex-M3 startup code
  \ include %CpuDir%/CodeM0M1		\ low level kernel definitions
  include %CommonDir%/messages          \ Umbilical messages
  include %CpuDir%/minM0M1              \ main code defs.
  include %CommonDir%/UFkernel          \ Forth high level kernel
  
  \ include %CommonDir%/kernel62          \ high level kernel definitions
  \ include %CommonDir%/Devtools		\ DUMP .S etc development tools
  \ include %CommonDir%/Voctools		\ ORDER VOCS etc
  \ include %CommonDir%/methods		\ target support for methods
  \ include %CpuDir%/LocalCM3		\ local variables
 

timebase? [if]
  include %CommonDir%/timebase		\ time base common code, MUST be before SysTickxxx
  include %CommonDir%/Delays		\ time delays
  ' start-timers AtCold
[else]
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

buildfile zero.no
l: version$
  build$,
l: BuildDate$
  DateTime$,

: ANS-FORTH	\ -- ; marker
;

include %AppDir%/interconnect

include %ZeroLib%/ZeroMemory	\ Various Addresses
include %ZeroLib%/MSC	  	\ Memory Controller

include %LocalCM3%/aapcs	\ AAPCS function calls

include %AppDir%/Delays
include %DriverDir%/serLE_pi \ polled serial driver

umbilical? [if]
  Synonym wait-byte  serkey0            \ Say which XTL drivers to use
  Synonym send-byte  seremit0
  Synonym Wait-Byte? serkey?0
  Synonym Init-XTL   Init-Ser
  include %CommonDir%/targend           \ message driver
[then]

\ *******************
\ *S Application code
\ *******************

include %AppDir%/Clock

: App           \ -- ; N.B. remove INIT-SER for interactive testing
[ init-idata? ] [if]  init-idata  [then]
[ umbilical? 0= ] [if]                  \ if final application
  init-ser                              \ initialise UART
  SerConsole opvec !                    \ select I/O devices
  SerConsole ipvec !
[then]
  $0A base c!                           \ default number conversion base
  cr cr ." MicroProcessor Engineering Umbilical Forth"
  begin
   cr  $0A 0
   do  i $030 + emit  loop             \ display numbers 0..9
   cr ." Press any key to continue " key drop
  again
  ;

umbilical? [if]
: runUmb        \ --
\ *G Starts Umbilical Forth with additional initialisation.
[ init-idata? ] [if]  init-idata [then]
   run-umbilical
;
[then]

\ ***************
\ *S Finishing up
\ ***************

libraries       \ to resolve common forward references
 include %CpuDir%/LibM0M1
 include %CommonDir%/ufLib
 include %LocalCM3%/CortexM0Atomic
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

umbilical? [if]
  make-turnkey runUmb                   \ cold start to Umbilical Forth
  \  finis
  c" /dev/cu.usbserial-FTFA7Z05" console0-speed serial       \ define link driver
  umbilical-forth                       \ switch to interactive mode
[else]
  make-turnkey App                      \ cold start to application
  finis
[then]

FINIS

\ ======
\ *> ###
\ ======

