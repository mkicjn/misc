#!/usr/bin/tclsh

source primitives.tcl

proc : {name args} {
	set ::colon($name) $args
}

: 1+ 1 + ;

proc dobody {body} {
	global prim colon
	for {set i 0} {$i < [llength $body]} {incr i} {
		set word [lindex $body $i]
		if {[info exists prim($word)]} {
			puts "Applying primitive $word"
			apply $prim($word)
		} elseif {[info exists colon($word)]} {
			puts "Interpreting colon definition $word"
			dobody $colon($word)
		} else {
			puts "Pushing string literal $word"
			push $word
		}
		incr index
	}
}

while {![eof stdin]} {
	dobody [gets stdin]
	puts "ok"
}
