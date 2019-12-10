#!/usr/bin/tclsh
global stack; set stack {}
proc push {args} {
	lappend ::stack {*}$args
}
proc pop {{n 1}} {
	global stack
	incr n -1
	set values [lrange $stack end-$n end]
	set stack [lreplace $stack end-$n end]
	return $values
}

proc bind {args} {
	set vals [pop [llength $args]]
	uplevel 1 [list lassign $vals {*}$args]
}

proc define {name} {
	set ::latest $name
	set ::imm($name) 0
}

proc prim {name body {imm ""}} {
	# Create a primitive with a given name and body
	define $name
	set ::prim($name) "{} {$body}"
	set ::imm($name) [expr {$imm ne ""}]
}

proc op2 {op {pre {}}} {
	# Create a primitive body representing a binary operator
	return "bind a b; push \[expr {${pre}(\$a $op \$b)}]"
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

prim /MOD {bind a b; push [expr {$a%$b}] [expr {$a/$b}]}

proc op1 {pre {post {}}} {
	# Create a primitive body representing a unary operator
	return "set a \[pop]; push \[expr {${pre}(\$a $post)}]"
}

prim INVERT [op1 ~]
prim NEGATE [op1 -]

prim DUP {bind a; push $a $a}
prim DROP pop
prim SWAP {bind a b; push $b $a}
prim ROT {bind a b c; push $b $c $a}

prim . {puts -nonewline "[pop] "}
prim CR {puts ""}

global line; set line [list]
proc word {} {
	global line
	set line [lassign $line word]
	return $word
}
prim REFILL {set ::line [gets stdin]; push [expr {-[eof stdin]}]}
prim PARSE-NAME {push [word]}

global latest; set latest {}
global imm; array set imm [list]
prim START-DEFINITION {define [pop]}
prim IMMEDIATE {set ::imm($::latest) 1}

prim ! {
	bind val name
	set i [lassign $name name]
	lset ::colon($name) {*}$i $val
}
prim @ {
	bind name
	set i [lassign $name name]
	push [lindex $::colon($name) {*}$i]
}
proc compile {args} {lappend ::colon($::latest) {*}$args}
prim , {compile [pop]}

prim BRANCH {
	uplevel 1 {
		incr i [lindex $colon($name) $i]
	}
}
prim 0BRANCH {
	if {[pop] == 0} {
		tailcall apply $::prim(BRANCH)
	} else {
		uplevel 1 {incr i}
	}
}
prim EXIT {return -code break}
prim LIT {
	uplevel 1 {
		push [lindex $colon($name) $i]
		incr i
	}
}
prim COMPARE {bind a b; push [string compare $a $b]}
prim BYE exit

prim GO-TO {uplevel 1 {lassign [lindex $colon($name) $i] name i}}
prim <BUILDS {
	define [word]
	compile LIT [list $::latest 4]
	compile EXIT ""
}
prim DOES> {
	uplevel 1 {
		lset ::colon($::latest) 2 GO-TO
		lset ::colon($::latest) 3 [list $name $i]
		return -code break
	}
}
prim SEE {puts $::colon([word])}
prim .S {puts "<[llength $::stack]> $::stack"}

proc execute {name} {
	global prim colon
	if {[info exists prim($name)]} {
		uplevel 1 [list apply $prim($name)]
	} elseif {[info exists colon($name)]} {
		set i 0
		while {[set word [lindex $colon($name) $i]] ne ""} {
			incr i
			set errcode [catch {execute $word} errmsg]
			if {$errcode == 0} continue
			if {[string first break $errmsg] >= 0} break
			error "$name->$word: $errmsg"
		}
	} else {
		error "$name?"
	}
}

global state; set state 0
prim \] {set ::state 1}
prim \[ {set ::state 0} immediate
prim : {define [word]; set ::state 1}
prim \; {compile EXIT; set ::state 0} immediate
prim INTERPRET-LINE {
	global state imm
	while {[llength $::line]} {
		set name [word]
		if {[string is integer $name]} {
			if {$state} {
				compile LIT $name
			} else {
				push $name
			}
		} elseif {[info exists imm($name)]} {
			if {$state && !$imm($name)} {
				compile $name
			} else {
				execute $name
			}
		} else {
			error "$name?"
		}
	}
}
prim QUIT {
	global prim
	while {1} {
		apply $prim(REFILL)
		if {[pop]} break
		if {[catch {apply $prim(INTERPRET-LINE)} err]} {
			puts $err
		} elseif {$::state} {
			puts "compiled"
		} else {
			puts "ok"
		}
	}
}

if {$tcl_interactive} return
apply $prim(QUIT)
#foreach name [array names colon] {
#	puts ": $name $colon($name) ; [expr {$imm($name)?"IMMEDIATE":""}]"
#}
