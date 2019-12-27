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

rename unknown -unknown
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
		-unknown {*}$args
	}
}

proc prim {name args} {set ::forth($name) $args; return}

proc effect {args script} {
	push {*}[apply [list $args $script] {*}[pop [llength $args]]]
	return
}

prim .    effect {a}     {puts -nonewline "$a "; flush stdout}
prim +    effect {a b}   {expr {$a+$b}}
prim >    effect {a b}   {expr {-($a>$b)}}

prim DUP  effect {a}     {list $a $a}
prim DROP effect {a}     {list}
prim SWAP effect {a b}   {list $b $a}
prim OVER effect {a b}   {list $a $b $a}
prim NIP  effect {a b}   {list $b}
prim TUCK effect {a b}   {list $b $a $b}
prim ROT  effect {a b c} {list $b $c $a}

proc interpret {words} {
	for {set ip 0} {$ip < [llength $words]} {incr ip} {
		eval [lindex $words $ip]
	}
}

prim EXIT    tailcall uplevel 1 return
prim LIT     uplevel 1 {push [lindex $words [incr ip]]}
prim BRANCH  uplevel 1 {incr ip [lindex $words [incr ip]]; incr ip -1}
prim 0BRANCH uplevel 1 {if {![pop]} {BRANCH} else {incr ip}}

if {$tcl_interactive} return
interpret {LIT 0 DUP . LIT 1 + DUP LIT 100 > 0BRANCH -10 DROP EXIT LIT 0 .}
