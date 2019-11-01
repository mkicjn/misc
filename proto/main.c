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
	tos = *(cell_t *)ip++;
	goto next;

dup_code: /* : DUP => dup */
	PUSH(sp) = tos;
	goto next;
drop_code: /* : DROP => drop */
	tos = POP(sp);
	goto next;
swap_code: /* : SWAP => swap */
	w.c = sp[-1];
	sp[-1] = tos;
	tos = w.c;
	goto next;
rot_code: /* : ROT => rot */
	w.c = sp[-2];
	sp[-2] = sp[-1];
	sp[-1] = tos;
	tos = w.c;
	goto next;

over_code: /* : OVER => over */
	PUSH(sp) = tos;
	tos = sp[-2];
	goto next;
nip_code: /* : NIP => nip */
	POP(sp);
	goto next;
tuck_code: /* : TUCK => tuck */
	w.c = sp[-1];
	sp[-1] = tos;
	PUSH(sp) = w.c;
	goto next;
unrot_code: /* : -ROT => unrot */
	w.c = sp[-1];
	sp[-1] = sp[-2];
	sp[-2] = tos;
	tos = w.c;
	goto next;

#define OP2(a,b) tos = b(POP(sp) a tos); goto next;
add_code: OP2(+,) /* : + => add */
sub_code: OP2(-,) /* : - => sub */
mul_code: OP2(*,) /* : * => mul */
div_code: OP2(/,) /* : / => div */
lsh_code: OP2(<<,) /* : LSHIFT => lsh */
rsh_code: OP2(>>,) /* : RSHIFT => rsh */

gt_code: OP2(>,-) /* : > => gt */
gte_code: OP2(>=,-) /* : >= => gte */
lt_code: OP2(<,-) /* : < => lt */
lte_code: OP2(<=,-) /* : <= => lte */
eq_code: OP2(==,-) /* : = => eq */
neq_code: OP2(!=,-) /* : <> => neq */

#define OP1(a,b) tos = a(tos b); goto next;
neg_code: OP1(-,) /* : NEGATE => neg */
not_code: OP1(~,) /* : INVERT => not */
lsh1_code: OP1(,<<1) /* : 2* => lsh1 */
rsh1_code: OP1(,>>1) /* : 2/ => rsh1 */

gtz_code: OP1(-,>0) /* : 0> => gtz */
gtez_code: OP1(-,>=0) /* : 0>= => gtez */
ltz_code: OP1(-,<0) /* : 0< => ltz */
ltez_code: OP1(-,<=0) /* : 0<= => ltez */
eqz_code: OP1(-,==0) /* : 0= => eqz */
neqz_code: OP1(-,!=0) /* : 0<> => neqz */

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
