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

proc prim {name body} {
	dict set ::forth_wordlist $name [list {} $body]
}
proc bind {name ops args} {
	prim $name [subst {
		push ::stack {*}\[$args {*}\[pop ::stack $ops]]
	}]
}

foreach name [list + - * / MOD AND OR XOR LSHIFT RSHIFT] \
	op   [list + - * / %   &   |  ^   <<     >>    ] \
	{bind $name 2 tcl::mathop::$op}
bind .    1 apply {{a} {puts -nonewline "$a "; flush stdout}}
bind BYE  0 exit
bind CR   0 puts ""
bind DUP  1 apply {{a}     {list $a $a}}
bind DROP 1 apply {{a}     {list}}
bind SWAP 2 apply {{a b}   {list $b $a}}
bind ROT  3 apply {{a b c} {list $b $c $a}}

proc find {name} {
	global context
	foreach dictvar [lreverse $context] {
		global $dictvar
		set dict [set $dictvar]
		if {[dict exists $dict $name]} {
			return [dict get $dict $name]
			break
		}
	}
	error "$name?"
}
bind FIND 0 find

proc interpret {line} {
	if {[lindex $line 0] eq {}} {
		set vals [lassign $line args body]
		apply [list $args $body] {*}$vals
		return
	}
	global stack
	global context
	for {set ip 0} {$ip < [llength $line]} {incr ip} {
		set word [lindex $line $ip]
		interpret [find $word]
	}
}

global source tib
set tib ""
set source ::tib
proc refill {} {
	if {$::source eq "::tib" && ![eof stdin]} {
		set ::source ::tib
		set ::tib [gets stdin]
		return -1
	} else {
		return 0
	}
}
bind REFILL 0 refill

proc word {} {
	global source
	global $source
	set line [set $source]
	set name [lindex $line 0]
	set $source [lrange $line 1 end]
	return $name
}
bind WORD 0 word

global latest
set latest ""
proc compile {args} {
	set dictvar [lindex $::context end]
	global $dictvar
	dict lappend $dictvar $::latest {*}$args
}

global state
set state 0
proc immediate? {name} {
	#TODO
	return 0
}
proc quit {} {
	global stack source state
	set stack [list]
	while {[refill]} {
		while {[set name [word]] ne ""} {
			if {![catch {set def [find $name]} err]} {
				if {!$state || [immediate? $name]} {
					interpret $def
				} else {
					compile $name
				}
			} elseif {[string is double $name]} {
				if {$state} {
					compile LIT $name
				} else {
					push stack $name
				}
			} else {
				error $err
			}
		}
		puts " ok"
	}
}
bind QUIT 0 quit

if {$tcl_interactive} return
while {[catch quit err]} {puts $err}
