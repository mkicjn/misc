/*
 * Unnamed Lisp interpreter (WIP)
 *
 * The goal of this Lisp interpreter is to be simple and readable, motivated by the following desires:
 * - To use this mini project as hands-on experience to better understand McCarthy's metacircular evaluator
 * - To have a prototypical Lisp interpreter simple enough to eventually translate into Forth (as a fun challenge later)
 *
 * In terms of features, this implementation only aims to eventually provide the bare minimum to meaningfully support microKanren.
 * (It still falls a bit short of that currently - porting that over will be another learning project down the line.)
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

#ifdef DEBUG
#undef DEBUG
#define DEBUG(X) X
#else
#define DEBUG(X)
#endif


// TODO list
// * Numeric types
// * More checks and error messages


// **************** Top-level definitions ****************

// X macro: Built-in symbols for which a primitive will be defined
#define FOREACH_PRIM(X) \
	X("\004cons", l_cons) \
	X("\003car", l_car) \
	X("\003cdr", l_cdr) \
	X("\004atom", l_atom) \
	X("\003eq?", l_eq) \
	X("\005null?", l_null) \
	X("\004eval", l_eval)

// X macro: All built-in symbols (with or without a corresponding primitive)
#define FOREACH_SYMVAR(X) \
	X("\001t", l_t) \
	X("\005quote", l_quote) \
	X("\004cond", l_cond) \
	X("\006lambda", l_lambda) \
	X("\005macro", l_macro) \
	X("\006define", l_define) \
	X("\003let", l_let) \
	FOREACH_PRIM(X)

// Declare a function for each primitive
#define DECLARE_FUNC(SYM,ID) void *ID(void *args, void *env);
FOREACH_PRIM(DECLARE_FUNC)

// Declare character pointer variables for each built-in symbol
#define DECLARE_SYMVAR(SYM,ID) char *ID##_sym;
FOREACH_SYMVAR(DECLARE_SYMVAR)

// Declare a compile-time numeric index for each primitive
enum l_prim_e {
#define DEFINE_ENUM_VAL(SYM,ID) ID##_e,
	FOREACH_PRIM(DEFINE_ENUM_VAL)
	NUM_PRIMS
};

// Global Lisp environment (populated at runtime)
void *defines = NULL;

// Designated sentinel values
#define ERROR ((void *)-1LL)
#define INCOMPLETE ((void *)-2LL)


// **************** Memory regions and region-based type inference ****************

// Space for cons cells in the form of [cell 0 car, cell 0 cdr, cell 1 car, cell 1 cdr, ...]
void *cells[MAX_CELL_SPACE];
void **next_cell = cells;

// Space for interned symbols in the form of counted strings (first char is length), stored consecutively
char syms[MAX_SYM_SPACE];
char *next_sym = syms;

// Space for pointers to primitive functions
typedef void *(*l_prim_t)(void *args, void *env); // Function pointer type for Lisp primitives
l_prim_t prims[NUM_PRIMS] = {
#define DEFINE_PRIM_VAL(SYM,ID) ID,
	FOREACH_PRIM(DEFINE_PRIM_VAL)
};

// Space for pointers to primitive function names
char *prim_syms[NUM_PRIMS] = {
#define DEFINE_PRIM_NAME(SYM,ID) SYM,
	FOREACH_PRIM(DEFINE_PRIM_NAME)
};

// Macro for determining whether a pointer lies within a given array
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

// Convenience macros for cons
#define list1(a) cons(a, NULL)
#define list2(a, b) cons(a, list1(b))
#define list3(a, b, c) cons(a, list2(b, c))
#define list4(a, b, c, d) cons(a, list3(b, c, d))

void *car(void *l)
{
	if (!IN(l, cells))
		return ERROR;
	return *(void **)l;
}

void *cdr(void *l)
{
	if (!IN(l, cells))
		return ERROR;
	return *((void **)l+1);
}

// Convenience macros for car/cdr
#define caar(x) car(car(x))
#define cdar(x) cdr(car(x))
#define cadar(x) car(cdr(car(x)))
#define cadr(x) car(cdr(x))
#define caddr(x) car(cdr(cdr(x)))
#define cadddr(x) car(cdr(cdr(cdr(x))))

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
		char *s = prim_syms[(void **)x - (void **)prims];
		printf("{primitive: %.*s}", *s, s + 1);
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

char *intern(char *s)
{
	// Intern the newest symbol, pointed at by s
	// (i.e., return a pointer to a duplicate symbol and free down to s if one exists)
	for (char *cmp = syms; cmp < s; cmp += *cmp + 1) { // For each symbol (consecutive counted strings)
		if (memcmp(cmp, s, *s + 1) == 0) { // memcmp checks length byte first
			next_sym = s; // i.e., free s
			return cmp;
		}
	}
	return s;
}

void *symbol(void)
{
	// Parse a symbol (and intern it)
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
	// Parse a Lisp expression
	space();
	if (peek == '\'') { // Quoted expression
		next();
		space();
		return list2(l_quote_sym, read());
	} else if (peek == '(') { // List
		next();
		void *list = body();
		space();
		if (peek != ')')
			return ERROR;
		next();
		return list;
	} else { // Everything else
		void *sym = symbol();
		if (sym == ERROR)
			next(); // Skip problem characters
		return sym;
	}
}


// **************** Garbage collection ****************

// The inspiration to use copying garbage collection comes from the second SectorLISP writeup - credit to them for that
// It has been implemented from scratch here and commented based on my own understanding of copying GC

// According to the tinylisp paper, using SectorLISP-style GC shouldn't work
// TODO: Figure out why it seems to work here, or find a counterexample to prove it doesn't

void *copy(void *x, void *pre_eval, intptr_t cell_offset)
{
	// Copy an object, offsetting all cell pointers above pre_eval
	if (!IN(x, cells) || x < pre_eval) // No need to copy values below the pre-eval point
		return x;
	void *a = copy(car(x), pre_eval, cell_offset);
	void *d = copy(cdr(x), pre_eval, cell_offset);
	return cons(a, d) + cell_offset;
}

void gc(void **ret, void **env, void *pre_eval)
{
#ifndef DISABLE_GC
	// Copying garbage collection for a return value and the environment
	if (next_cell == pre_eval) // Ellide useless calls
		return;
	// Create a fresh copy of the return value elsewhere, offsetting cells to match their post-GC position
	void *pre_copy = next_cell;
	intptr_t diff = (intptr_t)pre_copy - (intptr_t)pre_eval;
	void *post_gc_env = copy(*env, pre_eval, -diff);
	void *post_gc_ret = copy(*ret, pre_eval, -diff);
	// Move the copy directly to the pre-eval position
	size_t copy_size = (intptr_t)next_cell - (intptr_t)pre_copy;
	memcpy(pre_eval, pre_copy, copy_size);
	// Correct next_cell to account for GC
	next_cell = pre_eval + copy_size;
	DEBUG(printf("GC: %d -> %d\n", (void **)pre_copy - cells, (void **)next_cell - cells);)
	*env = post_gc_env;
	*ret = post_gc_ret;
#else
	DEBUG(printf("No GC: %d\n", (void **)next_cell - cells);)
#endif
}


/* **************** Interpreter (modified for TCO) **************** */

// The implementation of TCO here aims to modify the original interpreter structure as little as possible.
// The basic idea here is to:
// - Add a new eval function that relies on an infinite loop to step through the evaluation
// - Modify the old eval, evcon, and apply to return an expression to continue from instead of calling eval, where possible
// The identifiers cont, envp, and INCOMPLETE signal where these modifications happened.

void *eval(void *x, void *env);

void *assoc(void *k, void *l)
{
	// Search for value of key k in association list l
	if (!l) {
		printf("\033[31mUnbound variable: ");
		print(k);
		printf("\033[m\n");
		return ERROR;
	}
	else if (k == caar(l)) // string interning allows ==
		return cdar(l);
	else
		return assoc(k, cdr(l));
}

void *evlis(void *l, void *env)
{
	// Map eval over list l (i.e., to form an argument list)
	if (!l)
		return NULL;
	else if (IN(l, cells))
		return cons(eval(car(l), env), evlis(cdr(l), env));
	else
		return eval(l, env); // Append value of dangling atom to list
}

void *pairlis(void *ks, void *vs, void *env)
{
	// Pair keys (ks) with values (vs) in environment env
	if (!ks)
		return env;
	else if (IN(ks, cells))
		return cons(cons(car(ks), car(vs)), pairlis(cdr(ks), cdr(vs), env));
	else
		return cons(cons(ks, vs), env); // Bind remaining values to dangling key
}

void *apply(void *f, void *args, void **cont, void **envp)
{
	// Apply function f to args in environment env (modified for TCO)
	if (IN(f, prims)) {
		l_prim_t *fp = f;
		return (*fp)(args, *envp);
	} else if (IN(f, cells)) {
		// Closures are structured as (args body env)
		// `env` is set to nil if there is no meaningful environment to close over
		// This signals for us to reference the global environment instead, permitting recursive function definitions
		// (Credit goes entirely to tinylisp for this idea)
		void *env = cadddr(f);
		*envp = pairlis(cadr(f), args, env ? env : defines);
		*cont = caddr(f);
		return INCOMPLETE;
	} else {
		return ERROR;
	}
}

void *evcon(void *xs, void *env)
{
	// Evaluate cond expressions xs in environment env (modified for TCO)
	while (IN(xs, cells)) {
		if (eval(caar(xs), env))
			return cadar(xs);
		xs = cdr(xs);
	}
	if (!xs)
		return NULL;
	return ERROR;
}

void *evlet(void *ls, void *x, void **envp)
{
	// Evaluate expression x with let bindings ls on top of environment env (modified for TCO)
	while (IN(ls, cells)) {
		*envp = cons(cons(caar(ls), eval(cadar(ls), *envp)), *envp);
		ls = cdr(ls);
	}
	return x;
}

void *eval_step(void **cont, void **envp)
{
	void *x = *cont, *env = *envp;
	// Evaluate expression x in environment env (modified for TCO)
	if (IN(x, syms)) {
		// Symbol -> return variable binding
		return assoc(x, env);
	} else if (IN(x, cells)) {
		// List -> check for special forms
		if (car(x) == l_quote_sym) { // quote -> do not eval
			return cadr(x);
		} else if (car(x) == l_cond_sym) { // cond -> call evcon
			*cont = evcon(cdr(x), env);
		} else if (car(x) == l_let_sym) { // let -> call evlet
			*cont = evlet(cadr(x), caddr(x), envp);
		} else if (car(x) == l_lambda_sym || car(x) == l_macro_sym) {
			// lambda/macro -> return with args, body, env; see apply() for env caveat
			return list4(car(x), cadr(x), caddr(x), env == defines ? NULL : env);
		} else { // No special form -> apply function or macro
			void *f = eval(car(x), env);
			if (car(f) == l_macro_sym) // macro -> don't eval arguments
				return apply(f, cdr(x), cont, envp);
			else
				return apply(f, evlis(cdr(x), env), cont, envp);
		}
	} else {
		// Nil or unknown -> return as-is
		return x;
	}
	return INCOMPLETE;
}

void *eval(void *x, void *env)
{
	DEBUG(static int level = 0; level++;)
	DEBUG(printf("%*sL%d eval: ", 4*level, "", level); print(x); printf("\n");)
	// Tail-call optimized eval
	void *pre_eval, *ret;
	for (;;) {
		pre_eval = next_cell;
		ret = eval_step(&x, &env);
		if (ret != INCOMPLETE)
			break;
		gc(&x, &env, pre_eval); // Collect garbage, keeping the continuation expression

#ifdef DISABLE_TCO
		// If TCO is disabled for testing purposes, eval recursively
		ret = eval(x, env);
		break;
#endif
		DEBUG(printf("%*sL%d step: ", 4*level, "", level); print(x); printf("\n");)
	}
	DEBUG(printf("%*sL%d result: ", 4*level, "", level); print(ret); printf("\n");)
	DEBUG(level--;)

	gc(&ret, &env, pre_eval); // Collect garbage, keeping the return value
	return ret;
}


// **************** Primitive functions ****************

void *l_cons(void *args, void *env)
{
	return cons(car(args), cadr(args));
}

void *l_car(void *args, void *env)
{
	return caar(args);
}

void *l_cdr(void *args, void *env)
{
	return cdar(args);
}

void *l_atom(void *args, void *env)
{
	if (IN(car(args), cells))
		return NULL;
	return l_t_sym;
}

void *l_eq(void *args, void *env)
{
	if (car(args) == cadr(args))
		return l_t_sym;
	return NULL;
}

void *l_null(void *args, void *env)
{
	if (car(args))
		return NULL;
	return l_t_sym;
}

void *l_eval(void *args, void *env)
{
	return eval(car(args), env);
}


// **************** REPL ****************

void *evald(void *x)
{
	// eval() with global definitions and permitting `define`
	if (IN(x, cells) && car(x) == l_define_sym) {
		defines = cons(cons(cadr(x), eval(caddr(x), defines)), defines);
		return cadr(x);
	} else {
		return eval(x, defines);
	}
}

int main()
{
	// Set up symbols and bindings for built-ins
#define COPY_SYM(SYM,ID) \
		ID##_sym = next_sym; \
		memcpy(next_sym, SYM, SYM[0] + 1); \
		next_sym += SYM[0] + 1;
	FOREACH_SYMVAR(COPY_SYM)

#define DEFINE_PRIM(SYM,ID) \
		defines = cons(cons(ID##_sym, &prims[ID##_e]), defines);
	FOREACH_PRIM(DEFINE_PRIM)

	// Read-eval-print loop
	void *nil = NULL;
	for (;;) {
		void *pre_eval = next_cell;
		print(evald(read()));
		printf("\n");
		gc(&nil, &defines, pre_eval); // Destroy return value and keep definitions
	}
}
