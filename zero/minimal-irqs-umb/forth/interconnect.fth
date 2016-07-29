\ Access to the interconnect things.
\ Its got to match interconnect.h

$20000000 equ ICROOT

struct /INTER	\ -- size
	ptr inter.jumptable
	ptr u0rxdata 
	int inter.ticks 
end-struct

: ticks icroot inter.ticks @ ; 

: jt icroot @ ; 

struct /jt
	ptr EnterEM2
end-struct

