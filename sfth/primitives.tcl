proc push {value} {
	global stack
	lappend stack $value
}

proc pop {} {
	global stack
	set value [lindex $stack end]
	set stack [lrange $stack 0 end-1]
	return $value
}

proc bind {args} {
	# Bind values on stack to names given by args
	foreach varname [lreverse $args] {
		uplevel "set $varname \[pop]"
	}
}

proc prim {name body} {
	# Create a prim with a given name and body
	uplevel "set prim($name) {{} {global stack; $body}}"
}

proc op2 {op {pre {}}} {
	# Create a prim body representing a binary operator
	return "bind a b; push \[expr {${pre}(\$a $op \$b)}]"
}

prim + [op2 +]
prim - [op2 -]
prim * [op2 *]
prim LSHIFT [op2 <<]
prim RSHIFT [op2 >>]
prim << [op2 -]
prim >> [op2 -]
prim /MOD {
	bind a b
	push [expr {$a % $b}]
	push [expr {$a / $b}]
}

prim . {
	bind a
	puts $a
}
