: rot32  dup 32 rshift  swap 32 lshift  or ;
: square  dup * ;
: round  ( y z x -- z y x' )  square rot tuck +  rot32 ;

: squares  ( ctr key -- u )
	tuck *  tuck +  over  ( y z x )
	round round round round
	nip nip ;


: rng  create , , does>  1 over +!  2@ squares ;

$deadbeef dup rng random

: emits  1- for  dup emit  8 rshift  next drop ;
: test  begin  random  4 emits  again ;
test
bye
