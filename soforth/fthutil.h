#ifndef FTHUTIL_H
#define FTHUTIL_H
/*
 * 	Utilities for use in Forth primitives
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// Make appropriate type aliases for cells
#if INTPTR_MAX > 0xFFFFFFFF
typedef __uint128_t udcell;
typedef __int128_t dcell;
typedef int64_t cell;
typedef uint64_t ucell;
#elif INTPTR_MAX > 0xFFFF
typedef uint64_t udcell;
typedef int64_t dcell;
typedef int32_t cell;
typedef uint32_t ucell;
#else
typedef uint32_t udcell;
typedef int32_t dcell;
typedef int16_t cell;
typedef uint16_t ucell;
#endif

#define COUNT(a) (sizeof(a)/sizeof(a[0]))

#ifdef NO_ASM
// NO_ASM causes an empty macro asm to be defined.
// This removes all asm uses, including for labels.
#define asm(...)
#endif

// FTHREGS lists the Forth registers for use as arguments.
#define FTHREGS \
	register void ***ip, /* Instruction pointer */ \
	register cell   *sp, /* Stack pointer */ \
	register cell   *rp, /* Return pointer */ \
	         cell   *dp, /* Data space pointer */ \
	register void  **wp, /* Working pointer (ITC) */ \
	register cell   tos  /* Top of stack */

// NEXT performs an indirect-threaded continuation using labels as values.
#define NEXT() goto **(wp = *(ip++))

// Generic stack operations, producing lvalues
#define PUSH(r) *(r++)
#define POP(r) *(--r)

// Generic swap and rotate macros
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
