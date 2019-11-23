global stack
set stack {}
proc push {value} {
	lappend ::stack $value
}
proc pop {} {
	global stack
	set value [lindex $stack end]
	set stack [lreplace $stack end end]
	return $value
}

proc bind {args} {
	# Bind values on stack to names given by args
	foreach var [lreverse $args] {
		uplevel 1 "set $var [pop]"
	}
}

proc prim {name body} {
	# Create a primitive with a given name and body
	set ::prim($name) "{} {$body}"
}

proc op2 {op {pre {}}} {
	# Create a primitive body representing a binary operator
	return "bind a b; push \[expr {${pre}(\$a$op\$b)}]"
}

prim + [op2 +]
prim - [op2 -]
prim * [op2 *]
prim / [op2 /]
prim % [op2 %]
prim LSHIFT [op2 <<]
prim RSHIFT [op2 >>]
prim < [op2 < -]
prim > [op2 > -]
prim = [op2 = -]

prim /MOD {
	bind a b
	push [expr {$a%$b}]
	push [expr {$a/$b}]
}

proc op1 {pre {post {}}} {
	# Create a primitive body representing a unary operator
	return "bind a; push \[expr {${pre}(\$a$post)}]"
}

prim INVERT [op1 ~]
prim NEGATE [op1 -]

prim DUP {
	bind a
	push $a
	push $a
}
prim DROP {
	pop
}
prim SWAP {
	bind a b
	push $b
	push $a
}

prim . {
	puts -nonewline "[pop] "
}
prim CR {
	puts {}
}

global line
set line [list]
prim REFILL {
	set ::line [gets stdin]
}
prim PARSE-NAME {
	global line
	set line [lassign $line v]
	push $v
}
prim START-WORD {
	set ::colon($::state) [list]
}

prim ! {
	bind val name
	global $name
	set $name $val
}
prim @ {
	bind name
	global $name
	push [set $name]
}
prim , {
	lappend ::colon($::state) [pop]
}

prim BRANCH {
	uplevel 1 {
		incr i [lindex $body $i]
	}
}
prim 0BRANCH {
	uplevel 1 {
		if {[pop] == 0} {
			apply $::prim(BRANCH)
		} else {
			incr i
		}
	}
}
