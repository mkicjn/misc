#include <stddef.h>
#include "fthdef.h"

#include "dict.h"

void bye_code(FTH_REGS)
{ //: BYE ( bye )
	return;
}

void dolit_code(FTH_REGS)
{ //: DOLIT ( dolit )
	PUSH(sp) = tos;
	tos = (cell_t)*(ip++);
	next(ip, sp, rp, w, tos);
}

void docol_code(FTH_REGS)
{ //: DOCOL ( docol )
	PUSH(rp) = (cell_t)ip;
	ip = (void *)(w.p + 1);
	next(ip, sp, rp, w, tos);
}

void exit_code(FTH_REGS)
{ //: EXIT ( exit )
	ip = (void *)POP(rp);
	next(ip, sp, rp, w, tos);
}

#define OP2(c,op) \
void c##_code(FTH_REGS) \
{ tos = POP(sp) op tos; next(ip, sp, rp, w, tos); }
OP2(add, +) //: + ( add )
OP2(sub, -) //: - ( sub )
OP2(mul, *) //: * ( mul )
OP2(div, /) //: / ( div )

void interp(void (**ip[])())
{
	cell_t sp[64];
	cell_t rp[32];
	register union workreg w = {.p = NULL};
	register cell_t tos = 0;
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
