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
	X("\003key", key) \
	X("\004emit", emit) \
	X("\001.", dot) \
	X("\004exit", exit) \
	X("\003dup", dup) \
	X("\004drop", drop) \
	X("\004swap", swap) \
	X("\0022*", mul2) \
	X("\0022/", div2) \
	X("\0021+", inc) \
	X("\0021-", dec) \
	X("\003max", max) \
	X("\003rot", rot) \
	X("\0020=", zeq) \
	X("\003bye", bye) \
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
{
	*PUSH(sp) = tos;
	tos = getchar();
	NEXT();
}

WORD(emit)
{
	putchar(tos);
	tos = *POP(sp);
	NEXT();
}

WORD(dot)
{
	printf("%ld\n", tos);
	tos = *POP(sp);
	NEXT();
}

WORD(docol)
{
	*PUSH(rp) = (intptr_t)(&ip[1]);
	ip = (intptr_t *)ip[0];
	NEXT();
}

WORD(exit)
{
	ip = (intptr_t *)(*POP(rp));
	NEXT();
}

WORD(dolit)
{
	*PUSH(sp) = tos;
	tos = ip[0];
	ip++;
	NEXT();
}

WORD(dup)
{
	*PUSH(sp) = tos;
	NEXT();
}

WORD(drop)
{
	tos = *POP(sp);
	NEXT();
}

WORD(swap)
{
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
{
	tos <<= 1;
	NEXT();
}

WORD(div2)
{
	tos >>= 1;
	NEXT();
}

WORD(inc)
{
	tos++;
	NEXT();
}

WORD(dec)
{
	tos--;
	NEXT();
}

WORD(max)
{
	intptr_t x = *POP(sp);
	if (x > tos)
		tos = x;
	NEXT();
}

WORD(rot)
{
	intptr_t a = tos;
	intptr_t b = sp[0];
	intptr_t c = sp[-1];
	tos = c;
	sp[0] = a;
	sp[-1] = b;
	NEXT();
}

WORD(jmp)
{
	ip = (intptr_t *)ip[0];
	NEXT();
}

WORD(jz)
{
	if (tos == 0) {
		ip = (intptr_t *)ip[0];
	} else {
		ip++;
	}
	tos = *POP(sp);
	NEXT();
}

WORD(zeq)
{
	tos = (tos == 0);
	NEXT();
}

WORD(bye)
{
	(void)dp;
	(void)ip;
	(void)sp;
	(void)rp;
	(void)tos;
	return;
}

WORD(dummy_literal)
{
	*PUSH(sp) = tos;
	tos = 0x0123456789abcdef;
	NEXT();
}
