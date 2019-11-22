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

proc shift {var} {
	upvar $var v
	set v [lassign $v r]
	return $r
}
