#!/usr/bin/awk -f
BEGIN {RS=" "; FS=" "}
/^[+-]?[0-9]+\n?$/ {stack[si++]=$0}
/^\+\n?$/ {
	b=stack[--si]
	a=stack[--si]
	stack[si++]=a+b
}
END {print stack[--si]}
