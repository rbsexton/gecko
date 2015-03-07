\ Access to the interconnect things.
\ Its got to match interconnect.h

$20000000 equ ICROOT

struct /INTER	\ -- size
	ptr jumptable
	ptr u0rxdata 
end-struct

: jt icroot @ ; 

struct /jt
	ptr EnterEM2
end-struct

