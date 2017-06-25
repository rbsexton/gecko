(( App Startup ))

0 value JT \ The Jumptable

\ -------------------------------------------
\ The word that sets everything up.
\ This runs before the intro banner, after
\ Init-multi.
\ -------------------------------------------
: StartApp
	hex
	init-dp @ dp ! \ Set the dictionary pointer so that we can function.
	\ I have no idea why I am doing this instead of the compilation system.

	1 getruntimelinks to jt
	." StartApp!" 

	4 [ SCSSCR _SCS + ] literal  ! \ Set deepsleep
			
;

\ ------------------------------------------
\ Application code
\ ------------------------------------------
struct tod
 1 field h
 1 field m
 1 field s
end-struct
 
udata
create hms tod allot
cdata

: +CAP ( n max -- n ) >R 1 + dup r> = if drop 0 then ; 

: ADVANCE ( tod -- )
  dup s c@ #60 +cap  2dup swap s c! \ Advance the seconds.  base n -- 
  if drop exit then
  
  dup m c@ #60 +cap  2dup swap m c! \ advance minutes
  if drop exit then

  dup h c@ #24 +cap  2dup swap h c! \ advance minutes

  2drop  
;

\ ----------------------------------------------
\ Tools for writing to the LCD
\ ----------------------------------------------
: LCD#! ( n -- ) jt LCD_# @ swap call1-- ;
variable thecount

: COUNT-WORD
  1 $10 wakereq \ Request a wake
  begin
    pause
    thecount dup  @ 1+ dup lcd#! swap ! 
    stop     
  again
;

((
task foo 
' count-word foo initiate
))

 


