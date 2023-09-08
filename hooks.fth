: goto >r ;
: traverse-list>  @ begin dup 0<> while dup r@ goto @ repeat drop rdrop ;

defer hook-action
: hook  create 0 , does> hook-action ;

: list-prepend  here -rot tuck @ , , ! ;
: execute-hook  traverse-list> cell+ @ execute ;

: upon     ['] list-prepend is hook-action ;
: trigger  ['] execute-hook is hook-action ;

hook test-hook
:noname ." test 1" cr ; upon test-hook
:noname ." test 2" cr ; upon test-hook
:noname ." test 3" cr ; upon test-hook
trigger test-hook
bye
