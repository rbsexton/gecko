(( App Startup ))


\ -------------------------------------------
\ The word that sets everything up
\ -------------------------------------------
: StartApp
	hex
	init-dp @ dp ! \ Set the dictionary pointer so that we can function.
	\ I have no idea why I am doing this instead of the compilation system.
	." StartApp!  " 
	." ICRoot@" icroot @ . cr
	\ $100 0 do icroot @ u0rxdata @ . loop cr 
	4 SCSSCR _SCS + ! \ Set deepsleep
	
		
;

