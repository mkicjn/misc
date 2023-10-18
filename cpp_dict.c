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

#define FORTH_REGS(X) intptr_t *dp, void (**ip)(X), intptr_t *sp, intptr_t *rp, intptr_t tos
// ^ Don't want to leave X blank for declarations because compiler will unnecessarily clear eax
#define NEXT() ((*ip)(dp, ip + 1, sp, rp, tos))
#define PUSH(sp) *(++sp)
#define POP(sp) *(sp--)

#define WORD(cid) void cid##_code(FORTH_REGS(FORTH_REGS()))


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
	X("KEY", key) \
	X("EMIT", emit) \
	X("DOCOL", docol) \
	X("EXIT", exit) \
	X("DOLIT", dolit) \
	X("DUP", dup) \
	X("DROP", drop) \
	X("SWAP", swap) \
	X("+", add) \
	X("-", sub) \
	X("LSHIFT", lsh) \
	X("RSHIFT", rsh) \
	X("$", hex) \
	X##_END

FOR_EACH_WORD(DECLARE_LINKS)
FOR_EACH_WORD(DEFINE_LINKS)


// Word definitions

WORD(key)
{
	PUSH(rp) = tos;
	tos = getchar();
	NEXT();
}

WORD(emit)
{
	putchar(tos);
	tos = POP(sp);
	NEXT();
}

WORD(docol)
{
	PUSH(rp) = (intptr_t)ip;
	ip = *(void **)ip;
	NEXT();
}

WORD(exit)
{
	ip = (void *)(POP(rp));
	NEXT();
}

WORD(dolit)
{
	PUSH(sp) = tos;
	tos = (intptr_t)(*(++ip));
	NEXT();
}

WORD(dup)
{
	PUSH(sp) = tos;
	NEXT();
}

WORD(drop)
{
	tos = POP(sp);
	NEXT();
}

WORD(swap)
{
	intptr_t x = tos;
	tos = sp[0];
	sp[0] = x;
	NEXT();
}

#define WORD_2OP(name, op) \
WORD(name) \
{ \
	tos = (POP(sp) op tos); \
	NEXT(); \
}
WORD_2OP(add, +)
WORD_2OP(sub, -)
WORD_2OP(lsh, <<)
WORD_2OP(rsh, >>)
