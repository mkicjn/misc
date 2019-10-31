#include <stddef.h>
#include "fthdef.h"

#include "dict.h"

void bye_code FTH_REGS
{ // : BYE ( bye )
	return;
}

void dolit_code FTH_REGS
{ // : DOLIT ( dolit )
	*(sp++) = tos;
	tos = (cell_t)*(ip++);
	next(ip, sp, rp, w, tos);
}

void docol_code FTH_REGS
{ // : DOCOL ( docol )
	*(rp++) = (cell_t)ip;
	ip = (void *)(w.p + 1);
	next(ip, sp, rp, w, tos);
}

void exit_code FTH_REGS
{ // : EXIT ( exit )
	ip = (void *)*(--rp);
	next(ip, sp, rp, w, tos);
}

#define ARITH2(c,op) \
void c##_code FTH_REGS \
{ tos op##= *(--sp); next(ip, sp, rp, w, tos); }
ARITH2(add, +) // : + ( add )
ARITH2(sub, -) // : - ( sub )
ARITH2(mul, *) // : * ( mul )
ARITH2(div, /) // : / ( div )

void interp(void (**ip[])())
{
	cell_t sp[64];
	cell_t rp[32];
	union workreg w = {.p = NULL};
	cell_t tos = 0;
	next(ip, sp, rp, w, tos);
}

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
