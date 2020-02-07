#ifndef FTHUTIL_H
#define FTHUTIL_H

#include <stddef.h>
#include <stdbool.h>

/*
 * 	Utilities for use in Forth primitives
 */

typedef long long int cell;

#define COUNT(a) (sizeof(a)/sizeof(a[0]))

#ifdef NO_ASM
// NO_ASM causes an empty macro asm to be defined.
// This removes all asm uses, including for labels.
#define asm(...)
#endif

#define FTHREGS \
	register void ***ip, \
	register cell   *sp, \
	register cell   *rp, \
	         cell   *dp, \
	register void  **wp, \
	register cell   tos

#define NEXT() goto **(wp = *(ip++))

#define PUSH(r) *(r++)
#define POP(r) *(--r)

#define SWAP(t,x,y) \
	do { \
		t tmp = (x); \
		(x) = (y); \
		(y) = tmp; \
	} while (0)

#define ROT(t,x,y,z) \
	do { \
		t tmp = (x); \
		(x) = (y); \
		(y) = (z); \
		(z) = tmp; \
	} while (0)


#endif
