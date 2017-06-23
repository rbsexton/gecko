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

