// Direct threaded code
// Continuation passing style

#include <stdio.h>
#include <stdint.h>


// VM details

struct link {
	const char *name;
	void (*code)();
	struct link *next;
};

#define PUSH(sp) (++sp)
#define POP(sp) (sp--)

#define FORTH_ARGS(X) intptr_t *dp, intptr_t *ip, intptr_t *sp, intptr_t *rp, intptr_t tos
// ^ Don't want to leave X empty for declarations because compiler will assume variadicity
#define FORTH_REGS dp, ip, sp, rp, tos

#define FORTH_DECL(X) void X(FORTH_ARGS(FORTH_ARGS()))
#define WORD(cid) FORTH_DECL(cid##_code)
typedef FORTH_DECL((*dtc_t));

// Compile-time dictionary generation

#define DECLARE_LINKS_START
#define DECLARE_LINKS(nm, cid) \
	struct link cid##_link;
#define DECLARE_LINKS_END

#define DEFINE_LINKS_START struct link *latest = {
#define DEFINE_LINKS(nm, cid) &cid##_link }; \
	WORD(cid); \
	struct link cid##_link = { \
		.name = nm, \
		.code = cid##_code, \
		.next =
#define DEFINE_LINKS_END NULL };


// Dictionary entries

#define FOR_EACH_WORD(X) \
	X##_START \
	X("key", key) \
	X("emit", emit) \
	X(".", dot) \
	X("exit", exit) \
	X("dup", dup) \
	X("drop", drop) \
	X("swap", swap) \
	X("2*", mul2) \
	X("2/", div2) \
	X("1+", inc) \
	X("1-", dec) \
	X("max", max) \
	X("rot", rot) \
	X("0=", zeq) \
	X("bye", bye) \
	X##_END

FOR_EACH_WORD(DECLARE_LINKS)
FOR_EACH_WORD(DEFINE_LINKS)

// Word definitions

WORD(next)
{
	dtc_t f = (dtc_t)ip[0];
	f(dp, &ip[1], sp, rp, tos);
}
#define NEXT() next_code(FORTH_REGS)

WORD(key)
{ //printf("key %p\n", rp);
	*PUSH(sp) = tos;
	tos = getchar();
	NEXT();
}

WORD(emit)
{ //printf("emit %p\n", rp);
	putchar(tos);
	tos = *POP(sp);
	NEXT();
}

WORD(dot)
{ //printf("dot %p\n", rp);
	printf("%ld\n", tos);
	tos = *POP(sp);
	NEXT();
}

WORD(docol)
{ //printf("docol %p\n", rp);
	*PUSH(rp) = (intptr_t)(&ip[1]);
	ip = (intptr_t *)ip[0];
	NEXT();
}

WORD(exit)
{ //printf("exit %p\n", rp);
	ip = (intptr_t *)(*POP(rp));
	NEXT();
}

WORD(dolit)
{ //printf("dolit %ld\n", (intptr_t)ip[0]);
	*PUSH(sp) = tos;
	tos = ip[0];
	ip++;
	NEXT();
}

WORD(dup)
{ //printf("dup %p\n", rp);
	*PUSH(sp) = tos;
	NEXT();
}

WORD(drop)
{ //printf("drop %p\n", rp);
	tos = *POP(sp);
	NEXT();
}

WORD(swap)
{ //printf("swap %p\n", rp);
	intptr_t a = tos;
	intptr_t b = sp[0];
	tos = b;
	sp[0] = a;
	NEXT();
}

#define WORD_2OP(name, op) \
WORD(name) \
{ \
	tos = (*POP(sp) op tos); \
	NEXT(); \
}
WORD_2OP(add, +)
WORD_2OP(sub, -)
WORD_2OP(lsh, <<)
WORD_2OP(rsh, >>)
WORD_2OP(gt, >)
WORD_2OP(and, &)

WORD(mul2)
{ //printf("mul2 %p\n", rp);
	tos <<= 1;
	NEXT();
}

WORD(div2)
{ //printf("div2 %p\n", rp);
	tos >>= 1;
	NEXT();
}

WORD(inc)
{ //printf("inc %p\n", rp);
	tos++;
	NEXT();
}

WORD(dec)
{ //printf("dec %p\n", rp);
	tos--;
	NEXT();
}

WORD(max)
{ //printf("max %p\n", rp);
	intptr_t x = *POP(sp);
	if (x > tos)
		tos = x;
	NEXT();
}

WORD(rot)
{ //printf("rot %p\n", rp);
	intptr_t a = tos;
	intptr_t b = sp[0];
	intptr_t c = sp[-1];
	tos = c;
	sp[0] = a;
	sp[-1] = b;
	NEXT();
}

WORD(jmp)
{ //printf("jmp %p\n", rp);
	ip = (intptr_t *)ip[0];
	NEXT();
}

WORD(jz)
{ //printf("jz %p\n", rp);
	if (tos == 0) {
		ip = (intptr_t *)ip[0];
	} else {
		ip++;
	}
	tos = *POP(sp);
	NEXT();
}

WORD(zeq)
{ //printf("zeq %p\n", rp);
	tos = (tos == 0);
	NEXT();
}

WORD(bye)
{ //printf("bye %p\n", rp);
	(void)dp;
	(void)ip;
	(void)sp;
	(void)rp;
	(void)tos;
	return;
}
