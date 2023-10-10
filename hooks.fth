: take  dup @ dup @ rot ! ;
: give  2dup @ swap ! ! ;
\ ^ Reused from pool.fth

: goto >r ;
: traverse-list>  begin  @ dup 0<> while  dup r@ goto  repeat drop rdrop ;
\ ^ Use 2rdrop or unloop to exit

defer hook-action
: hook  create 0 , does>  hook-action ;

: prepend-hook  here swap  0 , rot ,  give ;
: execute-hook  traverse-list> cell+ @ execute ;

: upon     ['] prepend-hook is hook-action ;
: trigger  ['] execute-hook is hook-action ;

hook test-hook
:noname ." test 1" cr ; upon test-hook
:noname ." test 2" cr ; upon test-hook
:noname ." test 3" cr ; upon test-hook
trigger test-hook
bye
