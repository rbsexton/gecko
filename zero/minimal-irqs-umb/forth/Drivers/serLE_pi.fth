\ serLE_p.fth - Polled Driver for the low-energy UART.
\ Assumes that the hardware is already setup by the launcher.
\
\ Note - The assumption here is that the LEUART is running at 9600 baud
\ The Zero Gecko can wake in 2us, and a character takes 10ms at this baud rate.
\ So we might as well WFI if the FIFO is full.

\ ********************
\ *S Serial primitives
\ ********************

$8 equ LEUART_STATUS
$1C equ LEUART_RXDATA
$28 equ LEUART_TXDATA

internal

: +FaultConsole	( -- )  ;
\ *G Because this is a polled driver, *\fo{+FaultConsole} for
\ ** fault handling is a *\fo{NOOP}. See *\i{Cortex/FaultCortex.fth}
\ ** for more details.

: (seremit)	\ char base --
\ *G Transmit a character on the given UART.
  begin
    dup LEUART_STATUS + @ bit4 and 		\ Tx FIFO full test
  until
  LEUART_TXDATA + !
;

: (sertype)	\ caddr len base --
\ *G Transmit a string on the given UART.
  -rot bounds
  ?do  i c@ over (seremit)  loop
  drop
;

: (sercr)	\ base --
\ *G Transmit a CR/LF pair on the given UART.
  $0D over (seremit)  $0A swap (seremit)
;

: (serkey?)	\ base -- t/f
\ *G Return true if the given UART has a character avilable to read.
  @ -1 > 	\ Rx FIFO empty test
;

: (serkey)	\ base -- char
\ *G Wait for a character to come available on the given UART and
\ ** return the character.
  begin
[ tasking? ] [if]  pause  [then]
    dup @
     dup 0 < if [asm wfi asm] then 
     0 >= 
  until
  dup @ swap -1 swap !
;

: initUART	\ divisor22 base --
  drop drop
;

external


\ ********
\ *S UART0
\ ********

useUART0? [if]

: init-ser0	; 

: seremit0	\ char --
\ *G Transmit a character on UART0.
  _LEUART0 (seremit)  ;
: sertype0	\ c-addr len --
\ *G Transmit a string on UART0.
  _LEUART0 (sertype)  ;
: sercr0	\ --
\ *G Issue a CR/LF pair to UART0.
  _LEUART0 (sercr)  ;
: serkey?0	\ -- flag
\ *G Return true if UART0 has a character available.
  icroot u0rxdata (serkey?)  ;
: serkey0	\ -- char
\ *G Wait for a character on UART0.
  icroot u0rxdata (serkey)  ;
create Console0	\ -- addr ; OUT managed by upper driver
\ *G Generic I/O device for UART0.
  ' serkey0 ,		\ -- char ; receive char
  ' serkey?0 ,		\ -- flag ; check receive char
  ' seremit0 ,		\ -- char ; display char
  ' sertype0 ,		\ caddr len -- ; display string
  ' sercr0 ,		\ -- ; display new line
console-port 0 = [if]
  console0 constant console
\ *G *\fo{CONSOLE} is the device used by the Forth system for interaction.
\ ** It may be changed by one of the phrases of the form:
\ *C   <device>  dup opvec !  ipvec !
\ *C   <device>  SetConsole
[then]

[then]


\ ************************
\ *S System initialisation
\ ************************

: init-ser	\ --
\ *G Initialise all serial ports
  [ useUART0? ] [if]  init-ser0  [then]
;


\ ======
\ *> ###
\ ======

decimal
