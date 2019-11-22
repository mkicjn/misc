#!/usr/bin/tclsh

source primitives.tcl

set stack [list]
set line {}

while {![eof stdin]} {
	set line [split [gets stdin]]
	while {[llength $line]} {
		set line [lassign $line word]
		if {$word eq ""} continue
		if {[info exists prim($word)]} {
			apply $prim($word)
		} else {
			push $word
		}
	}
}
