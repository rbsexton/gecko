(( Declare needed things for the Zero Gecko ))

hex

40086000 equ _PCNT0

400CC000 constant _PRS
400C8000 constant _CMU
400C6000 constant _EMU
40084000 constant _LEUART0
40080000 constant _RTC
40010000 constant _TIMER0
40010400 constant _TIMER1


: .hms ( addr -- ) base @ swap decimal dup ? dup 4+ ? 8 + ? base ! ;
: hms ( -- ) icroot inter.tod.tra @ .hms ; 
: dhms ( -- ) icroot inter.tod.dec @ .hms ; 


decimal
