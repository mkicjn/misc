: xorshift+  dup 13 lshift xor  dup 7 rshift xor  dup 31 rshift + ;
: cbrng  $deadbeef tuck * tuck xor *  xorshift+ ;
: rng  create here cbrng , does>  1 over +!  @ cbrng ;

rng random

\ : test  for random u. cr next ;
\ hex 10 test

: emits  1- for  dup emit  8 rshift  next drop ;
: test  begin  random  8 emits  again ;
test
bye
