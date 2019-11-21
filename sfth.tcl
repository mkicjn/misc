#!/usr/bin/tclsh

proc push {stackvar value} {
	upvar $stackvar stack
	lappend stack $value
}

proc pop {stackvar} {
	upvar $stackvar stack
	set value [lindex $stack end]
	set stack [lreplace $stack end end]
	return $value
}

proc bind {stackvar args} {
	upvar $stackvar stack
	foreach varname $args {
		upvar $varname var
		set var [pop stack]
	}
}

proc primitive(+) {stackvar} {
	upvar $stackvar stack
	bind stack a b
	push stack [expr {$a+$b}]
}

set teststack [list 1 2 1 2]
primitive(+) teststack
puts $teststack

exit

while {![eof stdin]} {
	set line [gets stdin]
	foreach word [split $line] {
		if {$word eq ""} continue
		puts "$word"
	}
}
