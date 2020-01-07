#!/usr/bin/tclsh
global stack
set stack [list]
interp alias {} push {} lappend ::stack
proc pop {{n 1}} {
	incr n -1
	set vals    [lrange   $::stack end-$n end]
	set ::stack [lreplace $::stack end-$n end]
	return $vals
}
proc with {varlist script} {
	tailcall apply [list $varlist $script] {*}[pop [llength $varlist]]
}

rename unknown _unknown
proc unknown {args} {
	set cont [lassign $args word]
	if {[string is double $word]} {
		push $word
		tailcall {*}$cont
	} elseif {[info exists ::forth($word)]} {
		eval $::forth($word)
		tailcall {*}$cont
	} else {
		set ::stack [list]
		_unknown {*}$args
	}
}

set srcfilt {
	BEGIN  "; while {1} \{"
	WHILE  "; if {!\[pop]} break; "
	UNTIL  "; if {\[pop]} break\} "
	REPEAT "\}; "
	AGAIN  "\}; "

	IF     "; if {\[pop]} \{"
	ELSE   "\} else \{"
	THEN   "\}"
}
proc : {name args} {
	set ::forth($name) [string map $::srcfilt $args]
	return
}

: .    with {a}     {puts -nonewline "$a "; flush stdout}
: .S   eval         {puts "<[llength $::stack]> $::stack"}
: CR   eval         {puts ""}

: +    with {a b}   {push [expr   {$a + $b}]}
: -    with {a b}   {push [expr   {$a - $b}]}
: >    with {a b}   {push [expr {-($a > $b)}]}
: <    with {a b}   {push [expr {-($a < $b)}]}

: DUP  with {a}     {push $a $a}
: DROP with {a}     {push}
: SWAP with {a b}   {push $b $a}
: OVER with {a b}   {push $a $b $a}
: NIP  with {a b}   {push $b}
: TUCK with {a b}   {push $b $a $b}
: ROT  with {a b c} {push $b $c $a}

proc docol {args} {
	foreach word $args {set [incr i] $word}
	while {[incr ip] <= [llength $args]} {
		eval [set $ip]
	}
}

: EXIT    tailcall uplevel return
: LIT     uplevel {push [set [incr ip]]}
: BRANCH  uplevel {incr ip [set [incr ip]]; incr ip -1}
: 0BRANCH uplevel {if {![pop]} {BRANCH} else {incr ip}}

if {$tcl_interactive} return
: TEST 0 BEGIN DUP 20 < WHILE 1 + DUP . REPEAT DROP CR EXIT 0 . ;
TEST
