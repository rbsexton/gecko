(( Clock Code ))

\ The gadget that drives it all.   This is rigged for debugging,
\ terminate it by setting clockterm to non-zero.
: CLOCKSTART ( n -- ) 
  icroot inter.rtcsem 0 over ! \ Reset the free-running counter.
  swap \ addr n -- 
  0 do 
   dup @resetex! ?dup if advance else [asm wfi asm] then
  loop
  drop
;

: CLOCKINIT ( -- )
  #1000 needlemax
  2dup ! 4 + 2dup ! 4 +  ! 
;
 
\ Heres where we advance any needles.
: ADVANCE ( n -- )
  pwm0@ + pwm0! 
;

\ ----------------------------------------------------------
\ Needle Management
\ ----------------------------------------------------------
udata
create NEEDLEMAX 3 cells allot
cdata

: ++NEEDLE_S ; \ Called ever time.
: ++NEEDLE_M ; \ Every time we roll the seconds.

\ ----------------------------------------------------------
\ PWM/Timer Manipulation
\ ----------------------------------------------------------
_timer0 $34 + equ _PWM0 
_timer0 $44 + equ _PWM1 
_timer0 $54 + equ _PWM2 

: PWM0! ( n -- ) _pwm0 ! ;
: PWM1! ( n -- ) _pwm1 ! ;
: PWM2! ( n -- ) _pwm2 ! ;

: PWM0@ ( -- n ) _pwm0 w@ ;
: PWM1@ ( -- n ) _pwm1 w@ ;
: PWM2@ ( -- n ) _pwm2 w@ ;

: SCALE0 #1000 0 do I pwm0! loop ; 
: SCALE1 #1000 0 do I pwm0! [asm wfi asm] loop ; 

\ ----------------------------------------------------------
\ Keeping track of the time.
\ ----------------------------------------------------------
struct _HMS
	int hms.subsec
	int hms.s
	int hms.m
	int hms.h
	int hms.maxsubsec \ Max Value for subseconds
	int hms.maxsec   \ Max Value for secs+minutes
	int hms.maxhour   \ Max for hours
	ptr hms.w_s    \ Increment the seconds value
	ptr hms.w_m    \ increment the minutes + hours
end-struct

idata
create HMS  
  0 , 0 , 0 , 0 , \ Running Counters
  #16 , #60 , #24 , \ Limits
  ' ++needle_s , ' ++needle_m , \ Words to invoke
create DHMS
  0 , 0 , 0 , 0 , \ Running Counters
  #10 , #100 , #10 , \ Limits
' ++needle_s , ' ++needle_m , \ Words to invoke
cdata 

\ Core concept - We update the seconds 
\ each time this gets called.  
\ If the seconds wrap, we update the minutes
\ and the hours.
\ UNTESTED
: ADVANCETIME ( struct-addr -- )
  dup hms.w_s @ execute \ Invoke the official update word.
  1 over hms.subsec +!  \ Increment
   dup hms.subsec @
   over hms.maxsubsec @ < if drop exit then \ If less then max, we're done
  0 over hms.subsec ! \ Reset the subsecs 

  dup hms.w_m @ execute
  1 over hms.s +!
   dup hms.s @ over hms.maxsec @ < if drop exit then 
  0 over hms.s ! \ Reset the seconds

  1 over hms.m +!
   dup hms.m @ over hms.maxsec @ < if drop exit then 
  0 over hms.m ! \ Reset the minutes

  1 over hms.h +!
   dup hms.h @ over hms.maxhour @ < if drop exit then 
  0 over hms.h ! \ Reset the minutes

  drop 

;

: ? @ . ; 
: .hms ( addr -- ) 
  dup hms.h ?
  dup hms.m ?
  dup hms.s ?
  hms.subsec ?
  cr 
;

\ : foo 0 do hms advancetime loop ; 

\ ============================ UNTESTED =======================
\ VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

