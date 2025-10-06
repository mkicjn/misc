#include <stdio.h>
#include <stdint.h>

enum token {
	TOK_BYE,
	TOK_KEY,
	TOK_EMIT,
	TOK_DOCOL,
	TOK_EXIT,
	TOK_DOLIT,
	TOK_DUP,
	TOK_DROP,
	TOK_DEC,
	TOK_INC,
	TOK_ADD,
	TOK_MUL2,
	TOK_DIV2,
	TOK_SWAP,
	TOK_JZ,
	TOK_JMP,
	TOK_ZEQ,
	TOK_ROT,
	TOK_AND,
	TOK_MAX,
	TOK_GT,
	TOK_DOT,
};

#define PUSH(x) (++x)
#define POP(x)  (x--)

static intptr_t stack[1024], rstack[1024];
intptr_t tt_interp(const intptr_t *prog)
{
	register const intptr_t *ip = prog;
	register intptr_t *sp = stack;
	register intptr_t *rp = rstack;
	register intptr_t tos = 0;
	register intptr_t wp = 0;
	register intptr_t rot = 0;

	for (;;) {
		switch (*(ip++)) {
		case TOK_BYE: asm("bye:");
			return tos;
		case TOK_KEY: asm("key:");
			*PUSH(sp) = tos;
			tos = getchar();
			break;
		case TOK_EMIT: asm("emit:");
			putchar(tos);
			tos = *POP(sp);
			break;
		case TOK_DOCOL: asm("docol:");
			*PUSH(rp) = (intptr_t)(ip+1);
			ip = *(void **)ip;
			break;
		case TOK_EXIT: asm("exit:");
			ip = (intptr_t *)(*POP(rp));
			break;
		case TOK_DOLIT: asm("dolit:");
			*PUSH(sp) = tos;
			tos = *(ip++);
			break;
		case TOK_DUP: asm("dup:");
			*PUSH(sp) = tos;
			break;
		case TOK_DROP: asm("drop:");
			tos = *POP(sp);
			break;
		case TOK_DEC: asm("dec:");
			tos--;
			break;
		case TOK_INC: asm("inc:");
			tos++;
			break;
		case TOK_ADD: asm("add:");
			tos += *POP(sp);
			break;
		case TOK_MUL2: asm("mul2:");
			tos <<= 1;
			break;
		case TOK_DIV2: asm("div2:");
			tos >>= 1;
			break;
		case TOK_SWAP: asm("swap:");
			wp = tos;
			tos = sp[0];
			sp[0] = wp;
			break;
		case TOK_JZ: asm("jz:");
			wp = *(ip++);
			ip = (tos == 0) ? (intptr_t *)wp : ip;
			tos = *POP(sp);
			break;
		case TOK_JMP: asm("jmp:");
			wp = *(ip++);
			ip = (intptr_t *)wp;
			break;
		case TOK_ZEQ: asm("zeq:");
			tos = (tos == 0);
			break;
		case TOK_ROT: asm("rot:");
			// tos = tos;
			wp = *POP(sp);
			rot = *POP(sp);
			*PUSH(sp) = wp;
			*PUSH(sp) = tos;
			tos = rot;
			break;
		case TOK_AND: asm("and:");
			tos &= *POP(sp);
			break;
		case TOK_MAX: asm("max:");
			wp = *POP(sp);
			tos = (wp > tos ? wp : tos);
			break;
		case TOK_GT: asm("gt:");
			tos = (*POP(sp) > tos);
			break;
		case TOK_DOT: asm("dot:");
			printf("%ld\n", tos);
			tos = *POP(sp);
			break;
		default: asm("unknown:");
			printf("Unknown opcode %ld\n", ip[-1]);
			return tos;
		}
	}
}

static intptr_t iter[] = {
	/* 0 */ TOK_DUP,
	/* 1 */ TOK_DOLIT, 1,
	/* 3 */ TOK_AND,
	/* 4 */ TOK_ZEQ,
	/* 5 */ TOK_JZ, (intptr_t)&iter[10],
	/* 7 */ TOK_DIV2,
	/* 8 */ TOK_JMP, (intptr_t)&iter[14],
	/* 10 */ TOK_DUP,
	/* 11 */ TOK_MUL2,
	/* 12 */ TOK_ADD,
	/* 13 */ TOK_INC,
	/* 14 */ TOK_EXIT
};

static intptr_t collatz[] = {
	/* 0 */ TOK_DOLIT, 0,
	/* 2 */ TOK_SWAP,
	/* 3 */ TOK_DUP,
	/* 4 */ TOK_DOLIT, 1,
	/* 6 */ TOK_GT,
	/* 7 */ TOK_JZ, (intptr_t)&collatz[16],
	/* 9 */ TOK_SWAP,
	/* 10 */ TOK_INC,
	/* 11 */ TOK_SWAP,
	/* 12 */ TOK_DOCOL, (intptr_t)&iter[0],
	/* 14 */ TOK_JMP, (intptr_t)&collatz[3],
	/* 16 */ TOK_DROP,
	/* 17 */ TOK_EXIT
};

static intptr_t maxlen[] = {
	/* 0 */ TOK_DOLIT, 0,
	/* 2 */ TOK_SWAP,
	/* 3 */ TOK_DUP,
	/* 4 */ TOK_DOCOL, (intptr_t)&collatz[0],
	/* 6 */ TOK_ROT,
	/* 7 */ TOK_MAX,
	/* 8 */ TOK_SWAP,
	/* 9 */ TOK_DEC,
	/* 10 */ TOK_DUP,
	/* 11 */ TOK_ZEQ,
	/* 12 */ TOK_JZ, (intptr_t)&maxlen[3],
	/* 14 */ TOK_DROP,
	/* 15 */ TOK_EXIT
};

static intptr_t program[] = {
	/* 0 */ TOK_DOLIT, 1000000,
	/* 2 */ TOK_DOCOL, (intptr_t)&maxlen[0],
	/* 4 */ TOK_DOT,
	/* 5 */ TOK_BYE
};

int main()
{
	tt_interp(program);
	return 0;
}
