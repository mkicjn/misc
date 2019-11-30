#!/usr/bin/tclsh

source primitives.tcl

proc execute {name} {
	global prim colon
	if {[info exists prim($name)]} {
		uplevel 1 [list apply $prim($name)]
	} elseif {[info exists colon($name)]} {
		set i 0
		while {[set word [lindex $colon($name) $i]] ne ""} {
			incr i
			set errcode [catch {execute $word} errmsg]
			if {$errcode == 0} continue
			if {$errcode == 3} break
			error "$name->$word: $errmsg"
		}
	} else {
		error "$name?"
	}
}

global state; set state 0
prim \] {set ::state 1}
prim \[ {set ::state 0} immediate
prim : {define [word]; set ::state 1}
prim \; {compile EXIT; set ::state 0} immediate
prim INTERPRET-LINE {
	global state imm
	while {[llength $::line]} {
		set name [word]
		if {[string is integer $name]} {
			if {$state} {
				compile DOLIT $name
			} else {
				push $name
			}
		} elseif {[info exists imm($name)]} {
			if {$state && !$imm($name)} {
				compile $name
			} else {
				execute $name
			}
		} else {
			error "$name?"
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
while {1} {
	catch {apply $prim(QUIT)} err
	puts $err
}
