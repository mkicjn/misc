#!/usr/bin/tclsh
global stack
set stack [list]
interp alias {} push {} lappend ::stack
proc pop {{n 1}} {
	incr n -1
	set vals    [lrange   $::stack end-$n end]
	set ::stack [lreplace $::stack end-$n end]
	return $vals
}

rename unknown -unknown
proc unknown {args} {
	set cdr [lassign $args car]
	if {[string is double $car]} {
		push $car
		tailcall {*}$cdr
	} elseif {[info exists ::forth($car)]} {
		eval $::forth($car)
		tailcall {*}$cdr
	} else {
		-unknown {*}$args
	}
}

set forth(+) {push [tcl::mathop::+ {*}[pop 2]]}
set forth(.) {puts -nonewline "[pop] "}

if {$tcl_interactive} return
set tcl_interactive 1
while {1} {
	if {[catch {eval [gets stdin]} err]} {
		set ::stack [list]
		puts $err
	} elseif {![eof stdin]} {
		puts " ok"
	} else break
}
