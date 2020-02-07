#include <stddef.h>
#include <stdbool.h>

typedef long long int cell;

struct fthdef {
	struct fthdef *prev;
	char *name;
	bool imm : 1;
	cell len : 8*sizeof(cell)-1;
	void *cf;
	void **data[];
};

#define COUNT(a) (sizeof(a)/sizeof(a[0]))

#ifdef NO_ASM
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

