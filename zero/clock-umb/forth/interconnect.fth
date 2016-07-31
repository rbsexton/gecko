\ Access to the interconnect things.
\ Its got to match interconnect.h

$20000000 equ ICROOT

struct /INTER	\ -- size
	ptr inter.jumptable
	ptr u0rxdata 
	int inter.ticks 
	int inter.rtcsem \ The RTC increments this every time. 
	int inter.rtcdsem  
end-struct

: ticks icroot inter.ticks @ ; 

: ICFN@ ( index -- addr ) 4 * icroot @ + @ ; 
: (MSC_ErasePage) 1 icfn@ ;  
: (MSC_WriteWord) 2 icfn@ ; 


