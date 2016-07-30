(( Zero Gecko Memory System Controller (MSC) ))

((
This code makes use of the interconnect tables
in order to find the address of the Silabs-Supplied
functions, and the aapcs words for calling them.

These words abstract the in-memory address of the user data
page, but use byte addressing.
))

udata
create ud_payload 1 cells allot   
cdata

$0FE0:0000 equ _USERDATA

: UDPAGE_ERASE ( addr -- )   _USERDATA + (msc_erasepage) swap call1-- ;
: UD@          ( addr -- n ) _USERDATA + @ ;
: UD!          ( n addr -- )
  _USERDATA + 
  swap ud_payload ! \ We have to write from ram..
  (msc_writeword) swap ud_payload 4 call3-- 
; 
 