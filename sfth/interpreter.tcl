#!/usr/bin/tclsh

source primitives.tcl

proc IMMEDIATE {} {
	set ::immediate($::latest) 1
}
proc : {name args} {
	set ::colon($name) $args
	set ::immediate($name) 0
}

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

if {$tcl_interactive} return
while {![eof stdin]} {
	dobody [gets stdin]
	puts "ok"
}
