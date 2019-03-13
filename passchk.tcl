#!/usr/bin/tclsh
if {!$argc} {exit 1}
set sum [string toupper [string range [exec echo -n [lindex $argv 0] | sha1sum] 0 39]]
set hlist [dict create {*}[split [exec curl -s -S https://api.pwnedpasswords.com/range/[string range $sum 0 4]] "\n:"]]
if {[catch {puts [dict get $hlist [string range $sum 5 39]]}]} {
	puts 0
}
