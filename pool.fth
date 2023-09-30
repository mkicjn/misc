: take  dup @ dup @ rot ! ;
: give  2dup @ swap ! ! ;
\ Note: ^ These could be used as generic linked list operators
: pool: ( cap size -- ) cell max create here 0 , rot dup allot
        over cell+ tuck + swap do i over give over +loop 2drop ;
: available  -1 begin 1+ swap @ tuck 0= until ;

\ e.g., a toy example of a cons cell pool below

: kb  10 lshift ;
: mb  20 lshift ;

1 kb constant cons-space
cons-space 2 cells pool: cons-cells

: car  @ ;
: cdr  cell+ @ ;
: cons  cons-cells take  tuck cell+ !  tuck ! ;

: atom?  cons-cells dup cons-space + within invert ;

: display  dup atom? if . exit then
           ." ( "
           begin dup car recurse cdr dup atom? until
           ." . "
	   recurse
           ." )" ;

: lisp-free dup atom? if  drop  exit then
            dup car over cdr recurse recurse
	    cons-cells give ;

cons-cells available . cr

3 2 1 0 cons cons cons
dup display cr
lisp-free

cons-cells available . cr

bye
