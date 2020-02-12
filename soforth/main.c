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
	asm("bye:");
	*sp = tos;
	return sp;
docol_c:
	asm("docol:");
	PUSH(rp) = (cell)ip;
	ip = (void ***)&wp[1];
	NEXT();
exit_c:
	asm("exit:");
	ip = (void ***)POP(rp);
	NEXT();
execute_c:
	asm("execute:");
	wp = (void **)tos;
	tos = POP(sp);
	goto **wp;
branch_c:
	asm("branch:");
	ip += *(cell *)ip;
	NEXT();
qbranch_c:
	asm("qbranch:");
	wp = (void **)tos;
	tos = POP(sp);
	if (!wp)
		goto branch_c;
	ip++;
	NEXT();

	// Stack manipulation
lit_c:
	asm("lit:");
	PUSH(sp) = tos;
	tos = *(cell *)ip++;
	NEXT();
dup_c:
	asm("dup:");
	PUSH(sp) = tos;
	NEXT();
drop_c:
	asm("drop:");
	tos = POP(sp);
	NEXT();
swap_c:
	asm("swap:");
	SWAP(cell, tos,sp[-1]);
	NEXT();
over_c:
	asm("over:");
	PUSH(sp) = tos;
	tos = sp[-2];
	NEXT();

	// Memory access
store_c:
	asm("store:");
	*(cell *)tos = POP(sp);
	tos = POP(sp);
	NEXT();
fetch_c:
	asm("fetch:");
	tos = *(cell *)tos;
	NEXT();
cstore_c:
	asm("cstore:");
	*(char *)tos = POP(sp);
	tos = POP(sp);
	NEXT();
cfetch_c:
	asm("cfetch:");
	tos = *(char *)tos;
	NEXT();

	// Numerical operations
add_c:
	asm("add:");
	tos += POP(sp);
	NEXT();
zlt_c:
	asm("zlt:");
	tos = (tos < 0) ? ~0 : 0;
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
