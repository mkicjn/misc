#!/usr/bin/tclsh

source primitives.tcl

proc : {name args} {
	set ::colon($name) $args
}

: 1- 1 - ;
: 0<= 0 < INVERT ;
: i. DUP 0<= 0BRANCH 6 1- DUP . BRANCH -8 ;

proc dobody {body} {
	global prim colon
	set i 0
	while {$i < [llength $body]} {
		set word [lindex $body $i]
		incr i
		if {[info exists prim($word)]} {
			#puts "Applying primitive $word"
			apply $prim($word)
		} elseif {[info exists colon($word)]} {
			#puts "Interpreting colon definition $word"
			dobody $colon($word)
		} else {
			#puts "Pushing string literal $word"
			push $word
		}
	}
}

while {![eof stdin]} {
	dobody [gets stdin]
	puts "ok"
}
