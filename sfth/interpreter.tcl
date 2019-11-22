#!/usr/bin/tclsh

source primitives.tcl

global stack
set stack [list]

while {![eof stdin]} {
	set line [gets stdin]
	foreach word [split $line] {
		if {$word eq ""} continue
		if [info exists prim($word)] {
			apply $prim($word)
		} else {
			push $word
		}
	}
}
