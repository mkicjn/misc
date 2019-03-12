#!/usr/bin/tclsh
proc getch {} {
	exec stty raw <@ stdin
	set input [read stdin 1]
	exec stty cooked <@ stdin
	return $input
}
puts "^C to quit"
while {[getch] ne "\03"} {}
