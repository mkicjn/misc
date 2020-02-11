#include "fthutil.h"

#include "vocab.h"
VOCAB(DEF)

cell *engine(FTHREGS)
{
	static void **cfs[] = {VOCAB(CF)};

	if (ip == NULL) {
		struct fthdef *d = LAST_VOC;
		for (int i = COUNT(cfs)-1; i >= 0; i--) {
			d->cf = cfs[i];
			d = d->prev;
		}
		return NULL;
	}
	NEXT();

	// Control flow
bye_c:
	asm("bye_c:");
	*sp = tos;
	return sp;
docol_c:
	asm("docol_c:");
	PUSH(rp) = (cell)ip;
	ip = (void ***)&wp[1];
	NEXT();
exit_c:
	asm("exit_c:");
	ip = (void ***)POP(rp);
	NEXT();
execute_c:
	asm("execute_c:");
	wp = (void **)tos;
	tos = POP(sp);
	goto **wp;
branch_c:
	asm("branch_c:");
	ip += *(cell *)ip;
	NEXT();
qbranch_c:
	asm("qbranch_c:");
	wp = (void **)tos;
	tos = POP(sp);
	if (!wp)
		goto branch_c;
	ip++;
	NEXT();

	// Stack manipulation
lit_c:
	asm("lit_c:");
	PUSH(sp) = tos;
	tos = *(cell *)ip++;
	NEXT();
dup_c:
	asm("dup_c:");
	PUSH(sp) = tos;
	NEXT();
drop_c:
	asm("drop_c:");
	tos = POP(sp);
	NEXT();
swap_c:
	asm("swap_c:");
	SWAP(cell, tos,sp[-1]);
	NEXT();
rot_c: 
	asm("rot_c:");
	ROT(cell,sp[-2],sp[-1],tos);
	NEXT();

over_c:
	asm("over_c:");
	PUSH(sp) = tos;
	tos = sp[-2];
	NEXT();
nip_c:
	asm("nip_c:");
	(void)POP(sp);
	NEXT();
tuck_c:
	asm("tuck_c:");
	wp = (void **)sp[-1];
	sp[-1] = tos;
	PUSH(sp) = (cell)wp;
	NEXT();
unrot_c:
	asm("unrot_c:");
	ROT(cell, tos, sp[-1], sp[-2]);
	NEXT();
qdup_c:
	asm("qdup_c:");
	if (tos)
		PUSH(sp) = tos;
	NEXT();

	// Arithmetic
add_c:
	asm("add_c:");
	tos += POP(sp);
	NEXT();
}

void init_cfs(void)
{
	engine(0, 0, 0, 0, 0, 0);
}

int main(int argc, char *argv[])
{
	init_cfs();

	static void **test[] = {XT(cell),XT(dup),XT(add),XT(bye)};
	cell sp[16], rp[16];


	return *engine(test,sp,rp,0,0,0);
}
