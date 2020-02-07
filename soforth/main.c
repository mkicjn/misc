#include "fthutil.h"

#include "vocab.h"
VOCAB(DEF)

void engine(FTHREGS)
{
	static void **cfs[] = {VOCAB(CF)};

	if (ip == NULL) {
		struct fthdef *d = LAST_VOC;
		for (int i = 0; i < COUNT(cfs); i++) {
			d->cf = cfs[COUNT(cfs)-1-i];
			d = d->prev;
		}
		return;
	}
	NEXT();

bye_c:
	asm("bye_c:");
	return;
lit_c:
	asm("lit_c:");
	PUSH(sp) = tos;
	tos = *(cell *)ip++;
	NEXT();
exit_c:
	asm("exit_c:");
	ip = (void ***)POP(rp);
	NEXT();
docol_c:
	asm("docol_c:");
	PUSH(rp) = (cell)ip;
	ip = (void ***)&wp[1];
	NEXT();
swap_c:
	asm("swap_c:");
	SWAP(cell,tos,sp[-1]);
	NEXT();
add_c:
	asm("add_c:");
	tos += POP(sp);
	NEXT();
}

int main(int argc, char *argv[])
{
	return 0;
}
