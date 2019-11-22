#!/usr/bin/tclsh

proc push {stackvar value} {
	upvar $stackvar stack
	lappend stack $value
}

proc pop {stackvar} {
	upvar $stackvar stack
	set value [lindex $stack end]
	set stack [lrange $stack 0 end-1]
	return $value
}

proc bind {stackvarname stackvar args} {
	uplevel "upvar $stackvarname $stackvar"
	foreach varname $args {
		uplevel "set $varname \[pop $stackvar]"
	}
}

set primitive(+) {{stackvar} {
	bind $stackvar => a b
	push => [expr {$a+$b}]
}}
set primitive(.) {{stackvar} {
	bind $stackvar => a
	puts -nonewline "$a\n"
}}

set stack [list]

while {![eof stdin]} {
	set line [gets stdin]
	foreach word [split $line] {
		if {$word eq ""} continue
		if [info exists primitive($word)] {
			apply $primitive($word) stack
		} else {
			push stack $word
		}
	}
}
