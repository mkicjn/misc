source list_utils.tcl

proc bind {args} {
	# Bind values on stack to names given by args
	uplevel {global stack}
	foreach varname [lreverse $args] {
		uplevel "set $varname \[pop stack]"
	}
}

proc prim {name body} {
	# Create a primitive with a given name and body
	set ::prim($name) "{} {$body}"
}

proc op2 {op {pre {}}} {
	# Create a primitive body representing a binary operator
	return "bind a b; push stack \[expr {${pre}(\$a$op\$b)}]"
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
	push stack [expr {$a%$b}]
	push stack [expr {$a/$b}]
}

proc op1 {pre {post {}}} {
	# Create a primitive body representing a unary operator
	return "bind a; push stack \[expr {${pre}(\$a$post)}]"
}

prim INVERT [op1 ~]
prim NEGATE [op1 -]

prim . {
	bind a
	puts $a
}
