(( Declare needed things for the Zero Gecko ))

: hms ( -- ) icroot inter.tod.tra @ .hms ; 
: dhms ( -- ) icroot inter.tod.dec @ .hms ; 

\ Internal things

internal
: .hms ( addr -- ) base @ swap decimal dup ? dup 4+ ? 8 + ? base ! ;
external




decimal
