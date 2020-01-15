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

proc compile {args} {lappend ::forth($::last) {*}$args}
proc : {name args} {
	set ::last $name
	if {[lindex $args 0] in [info commands]} {
		set ::forth($::last) $args
		return
	}
	set ::forth($::last) docol
	foreach word $args {
		if {[string is double $word]} {
			compile LIT $word
		} elseif {[info exists ::imm($word)]} {
			eval $::forth($word)
		} else {
			compile $word
		}
	}
	compile EXIT
	return
}

proc docol {args} {
	foreach word $args {set [incr i] $word}
	while {[incr ip] <= [llength $args]} {
		eval $::forth([set $ip])
	}
}

: EXIT    return
: LIT     eval {push [set [incr ip]]}
: BRANCH  eval {incr ip [set [incr ip]]; incr ip -1}
: 0BRANCH eval {if {![pop]} {eval $::forth(BRANCH)} else {incr ip}}

: IMMEDIATE eval {set ::imm($::last) 1}
: , with {a} {compile $a}
: HERE eval {push [llength $::forth($::last)]}
: C! with {a b} {lset ::forth($::last) $b $a}

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

: MARK> HERE LIT UNRESOLVED , ;
: MARK< HERE ;
: >RESOLVE HERE OVER - SWAP C! ;
: <RESOLVE HERE - , ;

: IF LIT 0BRANCH , MARK> ; IMMEDIATE
: ELSE LIT BRANCH , MARK> SWAP >RESOLVE ; IMMEDIATE
: THEN >RESOLVE ; IMMEDIATE

: BEGIN MARK< ; IMMEDIATE
: WHILE LIT 0BRANCH , MARK> SWAP ; IMMEDIATE
: REPEAT LIT BRANCH , <RESOLVE >RESOLVE ; IMMEDIATE
: AGAIN LIT BRANCH , <RESOLVE ; IMMEDIATE
: UNTIL LIT 0BRANCH , <RESOLVE ; IMMEDIATE

if {$tcl_interactive} return
: TEST BEGIN DUP 0 > WHILE DUP . 1 - REPEAT EXIT . ;
puts $::forth(TEST)
100 TEST
