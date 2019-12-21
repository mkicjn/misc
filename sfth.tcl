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

global stack
set    stack [list]

global forth-wordlist
set    forth-wordlist [dict create]

global context
set    context [list forth-wordlist]

proc prim {name body} {
	global   forth-wordlist
	dict set forth-wordlist $name [list {} $body]
}
proc bind {name ops args} {
	prim $name [subst {
		global stack
		lpush stack {*}\[$args {*}\[lpop stack $ops]]
	}]
}

bind + 2 tcl::mathop::+
bind . 1 puts -nonewline

proc interpret {line} {
	global stack
	global context
	foreach word $line {
		set def ""
		foreach wlistvar $context {
			upvar 1 $wlistvar wlist
			if {![catch {set def [dict get $wlist $word]}]} break
		}
		if {$def ne ""} {
			apply $def
		} elseif {[string is entier $word]} {
			lpush stack $word
		} else {
			return -code error "$word?"
		}
	}
	puts " ok"
}

interpret [gets stdin]
