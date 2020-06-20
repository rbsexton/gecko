\ DELAYS.FTH - Customized to the launcher environment.

\ ticks is part of interconnect.
\ In this enviroment, everything is interrupt driven,
\ so doing a wfi ensures that time will pass.

: pause [asm wfi asm] ; 
: later ( -- n ) ticks + ;
: expired ( n -- t/f ) pause  ticks - 0< ; 

: ms ( n -- ) 
  later
  begin
    dup expired
  until
  drop
;
