/*
 * Unnamed Lisp interpreter (WIP)
 *
 * The goal of this Lisp interpreter is to be simple and readable, motivated by the following desires:
 * - To use this mini project as hands-on experience to better understand McCarthy's metacircular evaluator
 * - To have a prototypical Lisp interpreter simple enough to eventually translate into Forth (as a fun challenge later)
 *
 * In terms of features, this implementation only aims to eventually provide the bare minimum to meaningfully support microKanren.
 * (It falls far short of that currently - porting that over will be another learning project down the line.)
 *
 * Certain design decisions are inspired by Justine Tunney's SectorLISP and Dr. Robert van Engelen's tinylisp.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#ifndef MAX_CELL_SPACE
#define MAX_CELL_SPACE 100000
#endif

#ifndef MAX_SYM_SPACE
#define MAX_SYM_SPACE 100000
#endif


// TODO list
// * Eliminate prim() (and struct prim?)
// * Single quote parsing ('x)
// * Top-level environment (define)
// * Garbage collection for cons cells
// * TCO in some form or another
// * More error codes and typechecks (low priority)
// * Numeric types (low priority)


// **************** Top-level definitions ****************

#define ERROR ((void *)-1LL)

// Built-in symbols for which a primitive will be defined
#define FOREACH_PRIM(X) \
	X("\004cons", l_cons) \
	X("\003car", l_car) \
	X("\003cdr", l_cdr) \
	X("\004atom", l_atom) \
	X("\002eq", l_eq)

#define DECLARE_FUNC(SYM,ID) void *ID(void *args, void *env);
FOREACH_PRIM(DECLARE_FUNC)

// Primitive function structure
struct prim {
	char **name;
	void *(*func)(void *args, void *env);
};


// All built-in symbols
#define FOREACH_SYMVAR(X) \
	X("\001t", l_t) \
	X("\005quote", l_quote) \
	X("\004cond", l_cond) \
	X("\006lambda", l_lambda) \
	FOREACH_PRIM(X)

#define DECLARE_SYMVAR(SYM,ID) char *ID##_sym;
FOREACH_SYMVAR(DECLARE_SYMVAR)


// Memory regions

void *cells[MAX_CELL_SPACE]; // car, cdr; alternating
void **next_cell = cells;

char syms[MAX_SYM_SPACE]; // counted strings; consecutive
char *next_sym = syms;

struct prim prims[] = { // fixed size array of structs
#define DEFINE_STRUCT(SYM,ID) {.name = &ID##_sym, .func = ID},
	FOREACH_PRIM(DEFINE_STRUCT)
};


// **************** Region-based type inference ****************

#define IN(X,T) ((intptr_t)(X) >= (intptr_t)(T) \
		&& (intptr_t)(X) < (intptr_t)(T) + sizeof(T))


// **************** Basic value operations ****************

void *cons(void *x, void *y)
{
	void **car = next_cell++;
	void **cdr = next_cell++;
	*car = x;
	*cdr = y;
	return car;
}

static inline void *car(void *l)
{
	if (!IN(l, cells))
		return ERROR;
	return *(void **)l;
}

static inline void *cdr(void *l)
{
	if (!IN(l, cells))
		return ERROR;
	return *((void **)l+1);
}

char *intern(char *s)
{
	// Intern the newest symbol, pointed at by s
	// (i.e., return a pointer to a pre-existing equivalent symbol and free down to s if one exists)
	for (char *cmp = syms; cmp < s; cmp += *cmp + 1) { // For each symbol (consecutive counted strings)
		if (memcmp(cmp, s, *s + 1) == 0) { // if equal (memcmp checks length byte first)
			next_sym = s; // Free s
			return cmp;
		}
	}
	return s;
}

void print(void *x)
{
	if (!x) {
		printf("()");
	} else if (IN(x, cells)) {
		// For lists, first print the head
		printf("(");
		print(car(x));
		// Then print successive elements until encountering NIL or atom
		for (x = cdr(x); x && IN(x, cells); x = cdr(x)) {
			printf(" ");
			print(car(x));
		}
		// If encountering an atom, print with dot notation
		if (x) {
			printf(" . ");
			print(x);
		}
		printf(")");
	} else if (IN(x, syms)) {
		char *s = x;
		printf("%.*s", *s, s + 1);
	} else if (IN(x, prims)) {
		struct prim *p = x;
		printf("{primitive: %.*s}", *p->name, p->name + 1);
	} else {
		printf("\033[31m{error}\033[m");
	}
}


// **************** Parser ****************

void *read(void);

char peek = '\0';
char next(void)
{
	// Delay char stream by one to allow lookahead
	char c = peek;
	peek = getchar();
	return c;
}

void space(void)
{
	// Skip whitespace
	while (peek <= ' ') {
		if (peek == EOF) // Exit on EOF
			exit(0);
		next();
	}
}

void *body(void)
{
	// Parse a list body (i.e., without parentheses)
	space();
	if (peek == ')') {
		return NULL;
	} else if (peek == '.') {
		next(); // Discard .
		return read();
	} else {
		void *x = read();
		return cons(x, body());
	}
}

void *symbol(void)
{
	// Parse a symbol
	char *s = next_sym++;
	while (peek > ' ' && peek != '(' && peek != ')')
		*(next_sym++) = next();
	if (next_sym == s+1) // Disallow empty symbols
		return ERROR;
	*s = next_sym - (s+1); // Store length in first byte
	return intern(s);
}

void *read(void)
{
	space();
	if (peek == '(') {
		next(); // Discard (
		void *list = body();
		space();
		if (peek != ')')
			return ERROR;
		next(); // Discard )
		return list;
	} else {
		return symbol();
	}
}


/* **************** Interpreter **************** */

void *eval(void *x, void *env);

void *prim(void *s)
{
	// Search for a primitive by the name s
	if (!IN(s, syms))
		return NULL;
	for (int i = 0; i < sizeof(prims)/sizeof(*prims); i++) {
		if (s == *(prims[i].name))
			return &prims[i];
	}
	return NULL;
}

void *assoc(void *k, void *l)
{
	// Search for value of key k in association list l
	if (!l)
		return prim(k); // if not defined, see if it refers to a primitive
	else if (k == car(car(l))) // string interning allows ==
		return cdr(car(l));
	else
		return assoc(k, cdr(l));
}

void *evlis(void *l, void *env)
{
	// Map eval over list l (i.e., to form an argument list)
	if (IN(l, cells))
		return cons(eval(car(l), env), evlis(cdr(l), env));
	else // Support currying/variadicty by allowing dangling terms to be appended to an argument list
		return eval(l, env); // Currying support
}

void *pairlis(void *ks, void *vs, void *env)
{
	// Pair keys (ks) with values (vs) in environment env
	if (!ks)
		return env;
	if (!IN(ks, cells)) // Support currying/variadicity by binding remaining args to a dangling atom
		return cons(cons(ks, vs), env);
	return cons(cons(car(ks), car(vs)),
	         pairlis(cdr(ks), cdr(vs), env));
}

void *apply(void *f, void *args, void *env)
{
	// Apply function f to args in environment env
	if (IN(f, prims)) {
		struct prim *p = f;
		return p->func(args, env);
	} else if (IN(f, cells)) {
		// Closures (via lambda) are structured as:
		// ((args) (body) (env))
		return eval(car(cdr(f)), pairlis(car(f), args, car(cdr(cdr(f)))));
	} else {
		return ERROR;
	}
}

void *evcon(void *xs, void *env)
{
	// Evaluate cond expressions xs in environment env
	if (!xs)
		return NULL;
	if (eval(car(car(xs)), env))
		return eval(car(cdr(car(xs))), env);
	return evcon(cdr(xs), env);
}

void *lambda(void *args, void *body, void *env)
{
	// Closures (via lambda) are structured as:
	// ((args) (body) (env))
	return cons(args, cons(body, cons(env, NULL)));
}

void *eval(void *x, void *env)
{
	// Evaluate expression x in environment env
	if (IN(x, syms)) {
		return assoc(x, env);
	} else if (IN(x, cells)) {
		// Check for special forms
		if (car(x) == l_quote_sym)
			return car(cdr(x));
		else if (car(x) == l_cond_sym)
			return evcon(cdr(x), env);
		else if (car(x) == l_lambda_sym)
			return lambda(car(cdr(x)), car(cdr(cdr(x))), env);
		else
			return apply(eval(car(x), env), evlis(cdr(x), env), env);
	} else {
		return x;
	}
}


// **************** Primitives ****************

void *l_cons(void *args, void *env)
{
	return cons(car(args), car(cdr(args)));
}

void *l_car(void *args, void *env)
{
	return car(car(args));
}

void *l_cdr(void *args, void *env)
{
	return cdr(car(args));
}

void *l_atom(void *args, void *env)
{
	if (IN(car(args), cells))
		return NULL;
	return l_t_sym;
}

void *l_eq(void *args, void *env)
{
	if (car(args) == car(cdr(args)))
		return l_t_sym;
	return NULL;
}


// **************** REPL/Testing ****************

int main()
{
#define COPY_SYM(SYM,ID) \
		ID##_sym = next_sym; \
		memcpy(next_sym, SYM, SYM[0] + 1); \
		next_sym += SYM[0] + 1;
	FOREACH_SYMVAR(COPY_SYM)

	for (;;) {
		print(eval(read(), NULL));
		printf("\n");
	}
}
