(( App Startup ))


\ -------------------------------------------
\ The word that sets everything up
\ -------------------------------------------
: StartApp
	hex
	init-dp @ dp ! \ Set the dictionary pointer so that we can function.
	\ I have no idea why I am doing this instead of the compilation system.
	." StartApp! " cr 
	\ ICRoot@" $20000c00 @ . cr
;


