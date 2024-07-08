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
#include <time.h>

// TODO list
// * More primitives
//   * `define`
//   * Built-in symbols (`#t`)
// * Garbage collection
// * String interning
// * TCO in some form or another
// * More error codes and typechecks
// * Numbers

// **************** Memory regions ****************

#ifndef MAX_CELL_SPACE
#define MAX_CELL_SPACE 100000
#endif

void *cells[MAX_CELL_SPACE];
void **next_cell = cells;

#ifndef MAX_SYM_SPACE
#define MAX_SYM_SPACE 100000
#endif

char syms[MAX_SYM_SPACE];
char *next_sym = syms;

#define FOREACH_PRIM(X) \
	X("cons", l_cons) \
	X("car", l_car) \
	X("cdr", l_cdr)

struct prim {
	const char *name;
	void *(*func)(void *args, void *env);
};

#define DECLARE_FUNC(N,F) void *F(void *args, void *env);
FOREACH_PRIM(DECLARE_FUNC)

struct prim prims[] = {
#define DEFINE_STRUCT(N,F) {.name = N, .func = F},
	FOREACH_PRIM(DEFINE_STRUCT)
};

#define ERROR ((void *)-1LL)


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
	if (IN(l, cells))
		return *(void **)l;
	return ERROR;
}

static inline void *cdr(void *l)
{
	if (IN(l, cells))
		return *((void **)l+1);
	return ERROR;
}

bool eq(void *x, void *y)
{
	// Check equality between objects
	if (x == y)
		return true;
	if (IN(x, syms) && IN(y, syms))
		return (strcmp(x, y) == 0);
	else
		return false;
}

bool is_sym(void *x, const char *sym)
{
	return IN(x, syms) && strcmp(x, sym) == 0;
}

void display(void *x)
{
	if (!x) {
		printf("nil");
	} else if (IN(x, cells)) {
		// For lists, first print the head
		printf("(");
		display(car(x));
		// Then print successive elements until encountering NIL or atom
		for (x = cdr(x); x && IN(x, cells); x = cdr(x)) {
			printf(" ");
			display(car(x));
		}
		// If encountering an atom, print with dot notation
		if (x) {
			printf(" . ");
			display(x);
		}
		printf(")");
	} else if (IN(x, syms)) {
		printf("%s", (char *)x);
	} else if (IN(x, prims)) {
		printf("{primitive: %s}", ((struct prim *)x)->name);
	} else {
		printf("\033[31m{error}\033[m");
	}
}


// **************** Parser ****************

void *s_exp(void);

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

void *list(void)
{
	// Parse a list body after the opening parenthesis
	space();
	if (peek == ')') {
		next(); // Discard )
		return NULL;
	} else if (peek == '.') {
		next(); // Discard .
		void *x = s_exp();
		space();
		if (peek != ')')
			return ERROR;
		next(); // Discard )
		return x;
	} else {
		void *x = s_exp();
		return cons(x, list());
	}
}

void *symbol(void)
{
	// Parse a symbol
	char *s = next_sym;
	while (peek > ' ' && peek != '(' && peek != ')')
		*(next_sym++) = next();
	if (s == next_sym) // Disallow empty symbols
		return ERROR;
	*(next_sym++) = '\0';
	return s;
}

void *s_exp(void)
{
	space();
	if (peek == '(') {
		next(); // Discard (
		return list();
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
		if (strcmp(s, prims[i].name) == 0)
			return &prims[i];
	}
	return NULL;
}

void *assoc(void *k, void *l)
{
	// Search for value of key k in assoc list l
	if (!l)
		return prim(k);
	else if (eq(k, car(car(l))))
		return cdr(car(l));
	else
		return assoc(k, cdr(l));
}

void *evlis(void *l, void *env)
{
	// Map eval over list l
	if (IN(l, cells))
		return cons(eval(car(l), env), evlis(cdr(l), env));
	else
		return l;
}

void *pairlis(void *ks, void *vs, void *env)
{
	// Pair keys (ks) with values (vs) in environment env
	if (!ks)
		return env;
	else
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
		if (is_sym(car(x), "quote"))
			return car(cdr(x));
		else if (is_sym(car(x), "cond"))
			return evcon(cdr(x), env);
		else if (is_sym(car(x), "lambda"))
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

// TODO: More primitives are necessary


// **************** REPL/Testing ****************

int main()
{
	for (;;) {
		display(eval(s_exp(), NULL));
		printf("\n");
	}
}
