#!/usr/bin/tclsh

proc push {stackvar args} {
	upvar 1 $stackvar stack
	lappend $stackvar {*}$args
	return
}
proc pop {stackvar {n 1}} {
	upvar 1 $stackvar stack
	incr n -1
	set vals  [lrange   $stack end-$n end]
	set stack [lreplace $stack end-$n end]
	return $vals
}

global stack forth_wordlist context
set stack          [list]
set forth_wordlist [dict create]
set context        [list forth_wordlist]

proc prim {name args} {
	dict set ::forth_wordlist $name [list eval $args]
}
proc effect {args body} {
	push ::stack {*}[apply [list $args $body] {*}[pop ::stack [llength $args]]]
}

foreach name [list + - * / MOD AND OR XOR LSHIFT RSHIFT] \
	op   [list + - * / %   &   |  ^   <<     >>    ] \
	{prim $name effect {a b} "tcl::mathop::$op \$a \$b"}

prim BYE  effect {}      {exit}
prim CR   effect {}      {puts ""}
prim .    effect {a}     {puts -nonewline "$a "; flush stdout}
prim DUP  effect {a}     {list $a $a}
prim DROP effect {a}     {list}
prim SWAP effect {a b}   {list $b $a}
prim ROT  effect {a b c} {list $b $c $a}

global tib
set    tib [list]
proc word {} {
	while {![llength $::tib]} {
		if {[eof stdin]} exit
		set ::tib [gets stdin]
	}
	set word  [lindex $::tib 0]
	set ::tib [lrange $::tib 1 end]
	return $word
}

proc find {name} {
	global context
	foreach dictvar [lreverse $context] {
		global $dictvar
		set dict [set $dictvar]
		if {[dict exists $dict $name]} {
			return [list [dict get $dict $name] 0] ;# TODO: Immediates
			break
		}
	}
	error "$name?"
}

proc execute {def} {
	if {[lindex $def 0] in [list eval execute]} {
		tailcall {*}$def
	}
	global stack
	global context
	for {set ip 0} {$ip < [llength $def]} {incr ip} {
		set word [lindex $def $ip]
		execute [find $word]
	}
}

global latest
set latest ""
proc compile {args} {
	set dictvar [lindex $::context end]
	global $dictvar
	dict lappend $dictvar $::latest {*}$args
}

global state
set state 0
proc quit {} {
	set ::stack [list]
	while {1} {
		set name [word]
		if {![catch {lassign [find $name] def imm}]} {
			if {!$::state || $imm} {
				execute $def
			} else {
				compile $name
			}
		} elseif {[string is double $name]} {
			if {$::state} {
				compile LIT $name
			} else {
				push ::stack $name
			}
		} else {
			puts $err
			tailcall quit
		}
	}
}

if {$tcl_interactive} return
while {[catch {quit} err]} {puts $err}
