\ Access to the interconnect things.
\ Its got to match interconnect.h

$20000000 equ ICROOT

: ticks icroot inter.ticks @ ; 

struct /INTER	\ -- size
	ptr inter.jumptable
	ptr u0rxdata 
	int inter.ticks 
	int inter.tod.tra
	int inter.tod.dec 
end-struct

: jt icroot @ ; 

struct /jt
	ptr EnterEM2
end-struct

