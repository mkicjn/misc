: step  dup 1 and if  3 * 1+  else  2/  then ;
: len   0 >r  begin dup 1 > while  step r> 1+ >r  repeat drop r> ;
: maxlen  0 swap for  i len max  next ;

1000000 maxlen .
bye
