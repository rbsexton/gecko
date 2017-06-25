\ Access to the interconnect things.
\ Its got to match interconnect.h

struct /INTER	\ -- size
	ptr jumptable
	ptr u0rxdata 
end-struct

struct /jt
	ptr LCD_Wr
	ptr LCD_Alpha#Off
	ptr LCD_#
	ptr LCD_#Off	
	end-struct
	

