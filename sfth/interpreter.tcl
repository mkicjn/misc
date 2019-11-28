#!/usr/bin/tclsh

source primitives.tcl

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

global state; set state 0
prim EXECUTE {
	bind name
	if {!$::state || $::imm($name)} {
		tailcall dobody $name
	} else {
		compile $name
	}
}
prim \[ {set ::state 0} immediate
prim \] {set ::state 1}
prim : {
	set ::latest [word]
	set ::state 1
}
prim \; {
	compile EXIT
	set ::state 0
} immediate
prim .S {
	puts "<[llength $::stack]> $::stack"
}
prim INTERPRET-LINE {
	while {[llength $::line]} {
		set name [word]
		if {!$::state || ([info exists ::imm($name)] && $::imm($name))} {
			dobody $name
		} else {
			compile $name
		}
	}
}
prim QUIT {
	global prim
	while {1} {
		apply $prim(REFILL)
		if {[pop]} break
		apply $prim(INTERPRET-LINE)
		if {$::state} {
			puts "compiled"
		} else {
			puts "ok"
		}
	}
}

if {$tcl_interactive} return
apply $prim(QUIT)
