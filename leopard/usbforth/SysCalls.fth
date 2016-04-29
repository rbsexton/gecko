\ Wrappers for SAPI functions, ABI 4.0

\ Note that the system call number is embedded into the instruction,
\ so this is not so easily parameterized.

#0 equ SAPI_VEC_VERSION
#1 equ SAPI_VEC_01_GetRuntimeLinks
#2 equ SAPI_VEC_02_PutChar
#3 equ SAPI_VEC_03_GetChar
#4 equ SAPI_VEC_04_GetCharAvail
#5 equ SAPI_VEC_05_PutString
#6 equ SAPI_VEC_06_EOL
#14 equ SAPI_VEC_14_PetWatchdog
#15 equ SAPI_VEC_15_GetTimeMS

CODE SAPI-Version  \ -- n
\ *G Get the version of the binary ABI in use. 
	svc # SAPI_VEC_VERSION 
	str tos, [ psp, # -4 ] !
	mov tos, r0
	next,
END-CODE

CODE GETRUNTIMELINKS  \ type -- n
\ *G Get the runtime linking information.   Type 0 - A Dynamic linking 
\ ** table with name, address pairs.  Type 1 - A Zero-Terminated Jump table.
	mov r0, tos
	svc # SAPI_VEC_01_GetRuntimeLinks 
	str tos, [ psp, # -4 ] !
	mov tos, r0
	next,
END-CODE

CODE (seremitfc) \ char base --
\ *G Service call for a single char - This one has a special name because
\ ** It'll be wrapped by something that can respond to the flow control 
\ ** return code and PAUSE + Retry 
	mov r0, tos
	ldr r1, [ psp ], # 4
	mov r2, # 0 \ Don't request a wake.
	svc # SAPI_VEC_02_PutChar	
	mov tos, r0
    next,
END-CODE

CODE (serkeyfc) \ base -- char  
\ *G Get a character from the port, or -1 for fail
	mov r0, tos	
	mov r1, # 0 \ No blocking.
	svc # SAPI_VEC_03_GetChar
	mov tos, r0
	next,
END-CODE

CODE (serkey?) \ base -- t/f
\ *G Return true if the given stream has a character avilable to read.
\ The call returns the number of chars available.  
	mov r0, tos
	svc # SAPI_VEC_04_GetCharAvail
	mov tos, r0
	next,
END-CODE
 
\ These two can be created from simpler things, and are optional.
\ CODE (type) 
\ END-CODE

\ CODE (cr) 
\ END-CODE

CODE PetWatchDog  \ n --
\ *G Refresh the watchdog.  Pass in a platform-specific number
\ ** To specify a timerout (if supported), or zero for the default value. 
	mov r0, tos
	ldr r1, [ psp ], # 4
	svc # SAPI_VEC_14_PetWatchdog
	ldr tos, [ psp ], # 4
	next,
END-CODE

CODE TICKS  \ -- n 
\ *G The current value of the millisecond ticker.
	svc # SAPI_VEC_15_GetTimeMS
	str tos, [ psp, # -4 ] !
	mov tos, r0
	next,
END-CODE




