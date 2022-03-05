\ Object pools in 4 lines of Forth

: TAKE  DUP @ DUP @ ROT ! ;
: GIVE  2DUP @ SWAP ! ! ;
: POOL: ( cap size -- ) CELL MIN  CREATE HERE 0 ,  OVER ALLOT
  ROT OVER + OVER CELL+  DO  I OVER GIVE  OVER +LOOP 2DROP ;

\ e.g.

\ : KiB  10 LSHIFT ;
\ 10 KiB CELL POOL: OBJECTS
\ OBJECTS TAKE  INIT-OBJECT ...

\ TODO Consider adding an origin pointer to each region
\ in each pool to make free operations more universal
