#!/usr/bin/tclsh

source primitives.tcl

proc : {name args} {
	set ::colon($name) $args
}

: 1+ 1 + ;
: test 2 1+ . ;

proc docol {body} {
	global prim colon
	foreach word $body {
		if {[info exists prim($word)]} {
			puts "Applying primitive $word"
			apply $prim($word)
		} elseif {[info exists colon($word)]} {
			puts "Interpreting colon definition $word"
			docol $colon($word)
		} else {
			puts "Pushing string literal $word"
			push $word
		}
	}
}

while {![eof stdin]} {
	docol [gets stdin]
}
