#include "fthutil.h"

#include "vocab.h"
VOCAB(DEF)

void engine(FTHREGS)
{
	static void **cfs[] = {VOCAB(CF)};

	if (ip == NULL) {
		struct fthdef *d = LAST_VOC;
		for (int i = COUNT(cfs)-1; i >= 0; i--) {
			d->cf = cfs[i];
			d = d->prev;
		}
		return;
	}
	NEXT();

	// Control flow
bye_c:
	asm("bye_c:");
	return;
exit_c:
	asm("exit_c:");
	ip = (void ***)POP(rp);
	NEXT();

	// Non-primitive code fields
docol_c:
	asm("docol_c:");
	PUSH(rp) = (cell)ip;
	ip = (void ***)&wp[1];
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

	static void **test[] = {
		LIT(1),LIT(2),XT(add),
		XT(bye),
	};
	cell stack[16];

	engine(test,stack,0,0,0,0);
	return 0;
}
