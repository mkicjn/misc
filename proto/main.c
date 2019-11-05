#include <stddef.h>
#include "fthdef.h"

#ifdef USE_ENGINE_LABELS
#define ASMLABEL(x) __asm__(#x ":")
#else
#define ASMLABEL(x)
#endif

#define countof(a) (sizeof(a) / sizeof(a[0]))

#include "dict.c"

void engine(FTH_REGS)
{
	#include "cfas.c"
	if (!ip) {
		struct primitive *d = latest;
		for (size_t i = 0; i < countof(cfas); i++) {
			d->cfa = cfas[i];
			d = (struct primitive *)d->link.prev;
		}
		return;
	}

next: ASMLABEL(next);
	goto **(w.p = *(ip++));

bye_code: ASMLABEL(bye_code); /*: BYE ( bye ) ;*/
	return;

docol_code: ASMLABEL(docol_code); /*: DOCOL ( docol ) ;*/
	PUSH(rp) = (cell_t)ip;
	ip = (void ***)w.p + 1;
	goto next;
exit_code: ASMLABEL(exit_code); /*: EXIT ( exit ) ;*/
	ip = (void ***)POP(rp);
	goto next;
dolit_code: ASMLABEL(dolit_code); /*: DOLIT ( dolit ) ;*/
	PUSH(sp) = tos;
	tos = *(cell_t *)ip++;
	goto next;

dup_code: ASMLABEL(dup_code); /*: DUP ( dup ) ;*/
	PUSH(sp) = tos;
	goto next;
drop_code: ASMLABEL(drop_code); /*: DROP ( drop ) ;*/
	tos = POP(sp);
	goto next;
swap_code: ASMLABEL(swap_code); /*: SWAP ( swap ) ;*/
	w.c = sp[-1];
	sp[-1] = tos;
	tos = w.c;
	goto next;
rot_code: ASMLABEL(rot_code); /*: ROT ( rot ) ;*/
	w.c = sp[-2];
	sp[-2] = sp[-1];
	sp[-1] = tos;
	tos = w.c;
	goto next;

over_code: ASMLABEL(over_code); /*: OVER ( over ) ;*/
	PUSH(sp) = tos;
	tos = sp[-2];
	goto next;
nip_code: ASMLABEL(nip_code); /*: NIP ( nip ) ;*/
	(void)POP(sp);
	goto next;
tuck_code: ASMLABEL(tuck_code); /*: TUCK ( tuck ) ;*/
	w.c = sp[-1];
	sp[-1] = tos;
	PUSH(sp) = w.c;
	goto next;
unrot_code: ASMLABEL(unrot_code); /*: -ROT ( unrot ) ;*/
	w.c = sp[-1];
	sp[-1] = sp[-2];
	sp[-2] = tos;
	tos = w.c;
	goto next;

#define OP2(n,a,b) \
n##_code: ASMLABEL(n##_code); \
	tos = b(POP(sp) a tos); \
	goto next;
OP2(add,+,) /*: + ( add ) ;*/
OP2(sub,-,) /*: - ( sub ) ;*/
OP2(mul,*,) /*: * ( mul ) ;*/
OP2(div,/,) /*: / ( div ) ;*/
OP2(lsh,<<,) /*: LSHIFT ( lsh ) ;*/
OP2(rsh,>>,) /*: RSHIFT ( rsh ) ;*/

OP2(gt,>,-) /*: > ( gt ) ;*/
OP2(gte,>=,-) /*: >= ( gte ) ;*/
OP2(lt,<,-) /*: < ( lt ) ;*/
OP2(lte,<=,-) /*: <= ( lte ) ;*/
OP2(eq,==,-) /*: = ( eq ) ;*/
OP2(neq,!=,-) /*: <> ( neq ) ;*/

#define OP1(n,a,b) \
n##_code: ASMLABEL(n##_code); \
	tos = a(tos b); \
	goto next;
OP1(neg,-,) /*: NEGATE ( neg ) ;*/
OP1(not,~,) /*: INVERT ( not ) ;*/
OP1(lsh1,,<<1) /*: 2* ( lsh1 ) ;*/
OP1(rsh1,,>>1) /*: 2/ ( rsh1 ) ;*/

OP1(gtz,-,>0) /*: 0> ( gtz ) ;*/
OP1(gtez,-,>=0) /*: 0>= ( gtez ) ;*/
OP1(ltz,-,<0) /*: 0< ( ltz ) ;*/
OP1(ltez,-,<=0) /*: 0<= ( ltez ) ;*/
OP1(eqz,-,==0) /*: 0= ( eqz ) ;*/
OP1(neqz,-,!=0) /*: 0<> ( neqz ) ;*/

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
	(void)argc;
	(void)argv;
	engine(NULL, NULL, NULL, w0, 0);
	interp(&bye_def.cfa);
	return 0;
}
