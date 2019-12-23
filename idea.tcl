global stack
set stack [list]
interp alias {} push {} lappend ::stack
proc pop {{n 1}} {
	incr n -1
	set vals    [lrange   $::stack end-$n end]
	set ::stack [lreplace $::stack end-$n end]
	return $vals
}

rename unknown _unknown
proc unknown {args} {
	set cdr [lassign $args car]
	if {[string is double $car]} {
		push $car
		tailcall {*}$cdr
	} elseif {[info exists ::forth($car)]} {
		eval $::forth($car)
		tailcall {*}$cdr
	} else {
		_unknown {*}$args
	}
}

set forth(+) {lassign [pop 2] a b; push [expr {$a+$b}]}
set forth(.) {puts "[pop] "}

return
