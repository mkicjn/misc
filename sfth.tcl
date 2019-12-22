#!/usr/bin/tclsh

proc lpush {stackvar args} {
	upvar 1 $stackvar stack
	lappend $stackvar {*}$args
	return
}
proc lpop {stackvar {n 1}} {
	upvar 1 $stackvar stack
	incr n -1
	set vals  [lrange   $stack end-$n end]
	set stack [lreplace $stack end-$n end]
	return $vals
}

global stack forth_wordlist context
set    stack          [list]
set    forth_wordlist [dict create]
set    context        [list forth_wordlist]

proc prim {name body} {
	dict set ::forth_wordlist $name [list {} $body]
}
proc bind {name ops args} {
	prim $name [subst {
		lpush ::stack {*}\[$args {*}\[lpop ::stack $ops]]
	}]
}

foreach name [list + - * / MOD AND OR XOR LSHIFT RSHIFT] \
	op   [list + - * / %   &   |  ^   <<     >>    ] \
	{bind $name 2 tcl::mathop::$op}
bind .    1 apply {{a} {puts -nonewline "$a "; flush stdout}}
bind CR   0 puts ""
bind BYE  0 exit
bind DUP  1 apply {{a}     {list $a $a}}
bind DROP 1 apply {{a}     {list}}
bind SWAP 2 apply {{a b}   {list $b $a}}
bind ROT  3 apply {{a b c} {list $b $c $a}}

proc interpret {line} {
	if {[lindex $line 0] eq {}} {
		if {[llength $line] == 2} {apply $line}
		return
	}
	global stack
	global context
	foreach word $line {
		set def ""
		foreach wlistvar [lreverse $context] {
			upvar 1 $wlistvar wlist
			if {![catch {
				set def [dict get $wlist $word]
			}]} break
		}
		if {$def ne ""} {
			interpret $def
		} elseif {[string is double $word]} {
			lpush stack $word
		} else {
			return -code error "$word?"
		}
	}
}

if {$tcl_interactive} return
while {![eof stdin]} {
	if {[catch {interpret [gets stdin]} err]} {
		puts $err
	}
}
