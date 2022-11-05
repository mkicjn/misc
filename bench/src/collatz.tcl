proc collatz {n} {
	set l 0
	while {$n > 1} {
		if {$n % 2 == 0} {
			set n [expr {$n / 2}]
		} else {
			set n [expr {$n * 3 + 1}]
		}
		incr l
	}
	return $l
}

proc maxlen {lim} {
	set max 0
	for {set i 0} {$i < $lim} {incr i} {
		set l [collatz $i]
		if {$max < $l} {
			set max $l
		}
	}
	return $max
}

puts [maxlen 1000000]
