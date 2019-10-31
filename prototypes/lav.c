#include "fthdef.h"
#include <stddef.h>

int main(int argc, char **argv)
{
	static void **inc[] = {
		&&docol_code,
		dolit_def.xt, (void *)1,
		add_def.xt, exit_def.xt
	};
	static void **test[] = {
		dolit_def.xt, (void *)2,
		(void **)&inc,
		bye_def.xt
	};

	cell_t stack[64];
	cell_t rstack[32];

	register void ***ip = test;
	register union workreg w = {.p = NULL};
	register cell_t *sp = stack;
	register cell_t *rp = rstack;
	register cell_t tos = 0;

	// TODO: Generate dictionary entries, #include somewhere

next:
	goto **(w.p = *(ip++));

bye_code: //: BYE ( bye )
	return tos;

dolit_code: //: DOLIT ( dolit )
	PUSH(sp) = tos;
	tos = (cell_t) * (ip++);
	goto next;
docol_code: //: DOCOL ( docol )
	PUSH(rp) = (cell_t)ip;
	ip = (void ***)w.p + 1;
	goto next;
exit_code: //: EXIT ( exit )
	ip = (void ***)POP(rp);
	goto next;

#define OP2(op) tos op##= POP(sp); goto next;
add_code: OP2(+) //: + ( add )
sub_code: OP2(-) //: - ( sub )
mul_code: OP2(*) //: * ( mul )
div_code: OP2(/) //: / ( div )
}
