#ifndef FTHDEF_H
#define FTHDEF_H
#include <stdint.h>
#if INTPTR_MAX > 0xFFFFFFFF
typedef __uint128_t udcell_t;
typedef __int128_t dcell_t;
typedef int64_t cell_t;
typedef uint64_t ucell_t;
#elif INTPTR_MAX > 0xFFFF
typedef uint64_t udcell_t;
typedef int64_t dcell_t;
typedef int32_t cell_t;
typedef uint32_t ucell_t;
#else
typedef uint32_t udcell_t;
typedef int32_t dcell_t;
typedef int16_t cell_t;
typedef uint16_t ucell_t;
#endif

struct link {
	struct link *prev;
	char *name;
	cell_t namelen;
};

#define FTH_REGS \
	void **ip[], \
	cell_t *sp, \
	cell_t *rp, \
	union workreg w, \
	cell_t tos

union workreg {
	void **p;
	cell_t c;
};

const union workreg w0 = {.p = NULL};

struct primitive {
	struct link link;
	void *cfa;
	void **data[];
};

#define PUSH(x) *(x++)
#define POP(x) *(--x)
#endif
