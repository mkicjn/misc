proc push {value} {
	lappend ::stack $value
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

proc prim {cname name body} {
	# Create a primitive with a given name and body
	set ::prim($name) "{} {$body}"
	set ::ct($name) $cname
	set ::cfa($name) "&&${cname}_code"
	set ::data($name) [list]
}

proc op2 {op {pre {}}} {
	# Create a primitive body representing a binary operator
	return "bind a b; push \[expr {${pre}(\$a$op\$b)}]"
}
prim add + [op2 +]
prim sub - [op2 -]
prim mul * [op2 *]
prim div / [op2 /]
prim mod % [op2 %]
prim shl LSHIFT [op2 <<]
prim shr RSHIFT [op2 >>]
prim lt < [op2 < -]
prim gt > [op2 > -]
prim eq = [op2 = -]
prim divmod /MOD {
	bind a b
	push [expr {$a%$b}]
	push [expr {$a/$b}]
}

proc op1 {pre {post {}}} {
	# Create a primitive body representing a unary operator
	return "bind a; push \[expr {${pre}(\$a$post)}]"
}
prim not INVERT [op1 ~]
prim neg NEGATE [op1 -]

prim dot . {
	bind a
	puts $a
}
