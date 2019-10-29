#include <stdio.h>
#include <termios.h>
typedef long cell_t;

struct link {
	struct link *prev;
	char *name;
	cell_t namelen;
};

#define FTH_REGS ( \
		void (**ip[])(), \
		cell_t *sp, \
		cell_t *rp, \
		union workreg w, \
		cell_t tos \
	)
union workreg {
	void (**p) FTH_REGS;
	cell_t c;
};

struct primitive {
	struct link link;
	void (**xt[1]) FTH_REGS;
};

static inline void next FTH_REGS
{
	w.p = *(ip++);
	(*w.p)(ip, sp, rp, w, tos);
}

#define BL
#define POSTPONE(x) x BL
#define EVAL(...) __VA_ARGS__

#define NAMES_INIT() POSTPONE(DEF) (0,
#define PER_NAME_ENTRY(c,f) c, f) POSTPONE(DEF) (&c##_def,
#define NAMES_END() tail, "")
#define DEF(p,c,f) \
void c##_code FTH_REGS; \
struct primitive c##_def = { \
	.link = { \
		.prev = (struct link *)p, \
		.name = #f, \
		.namelen = sizeof(#f), \
	}, \
	.xt = {(void *)c##_code}, \
};
void tail_code FTH_REGS {}

#define NAMES(INIT,PER_ENTRY,END) \
	INIT() \
	PER_ENTRY(bye, BYE) \
	PER_ENTRY(dolit, DOLIT) \
	PER_ENTRY(docol, DOCOL) \
	PER_ENTRY(exit, EXIT) \
	PER_ENTRY(add, +) \
	PER_ENTRY(sub, -) \
	PER_ENTRY(mul, *) \
	PER_ENTRY(div, /) \
	END()

void bye_code FTH_REGS
{
	return;
}

void dolit_code FTH_REGS
{
	*(sp++) = tos;
	tos = (cell_t)*(ip++);
	next(ip, sp, rp, w, tos);
}

void docol_code FTH_REGS
{
	*(rp++) = (cell_t)ip;
	ip = (void *)(w.p + 1);
	next(ip, sp, rp, w, tos);
}

void exit_code FTH_REGS
{
	ip = (void *)*(--rp);
	next(ip, sp, rp, w, tos);
}

#define ARITH2(c,op) \
void c##_code FTH_REGS \
{ tos op##= *(--sp); next(ip, sp, rp, w, tos); }
ARITH2(add, +)
ARITH2(sub, -)
ARITH2(mul, *)
ARITH2(div, /)

void interp(void (**ip[])())
{
	cell_t sp[64];
	cell_t rp[32];
	union workreg w = {.p = NULL};
	cell_t tos = 0;
	next(ip, sp, rp, w, tos);
}

EVAL(NAMES(NAMES_INIT,PER_NAME_ENTRY,NAMES_END))

int main(int argc, char **argv)
{
	/* Temporary */
	typedef void (**f)();
	static void (**inc_xt[])() = {
		(f)docol_code, // !!
		(f)dolit_def.xt, (f)1,
		(f)add_def.xt,
		(f)exit_def.xt,
	};
	static void (**p[])() = {
		(f)dolit_def.xt, (f)2,
		(f)inc_xt,
		(f)bye_def.xt,
	};
	/* ********* */
	interp(p);
	return 0;
}
