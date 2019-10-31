#include <stddef.h>
#include "fthdef.h"

struct primitive *engine(FTH_REGS)
{
#include "dict.c"
	if (!ip)
		return latest;
next:
	goto **(w.p = *(ip++));

docol_code: /* : DOCOL => docol */
	PUSH(rp) = (cell_t)ip;
	ip = (void ***)w.p + 1;
	goto next;
exit_code: /* : EXIT => exit */
	ip = (void ***)POP(rp);
	goto next;
dolit_code: /* : DOLIT => dolit */
	PUSH(sp) = tos;
	tos = (cell_t) * (ip++);
	goto next;

#define OP2(op) tos = POP(sp) op tos; goto next;
add_code: OP2(+) /* : + => add */
sub_code: OP2(-) /* : - => sub */
mul_code: OP2(*) /* : * => mul */
div_code: OP2(/) /* : / => div */

bye_code: /* : BYE => bye */
	return NULL;
}

void execute(void **xt,cell_t *sp,cell_t *rp)
{
	static void *bye = &&ret;

	void **ip[2] = {xt, &bye};
	cell_t tos = POP(sp);

	engine(ip, sp, rp, w0, tos);
ret:
	PUSH(sp) = tos;
	return;
}

void interp(void **xt)
{
	cell_t stack[64];
	cell_t rstack[32];

	execute(xt, stack, rstack);
}

int main(int argc, char **argv)
{
	void **xt = &engine(NULL, NULL, NULL, w0, 0)->cfa;
	interp(xt);
	return 0;
}
