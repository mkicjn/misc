#!/usr/bin/awk -f
BEGIN {RS=" "}
/^-?[0-9]+\n?$/ {stack[si++]=int($0)}
/^\+\n?$/ {
	b=stack[--si]
	a=stack[--si]
	stack[si++]=a+b
}
/^\.\n?$/ {print stack[--si]}
