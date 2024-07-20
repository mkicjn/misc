/*
 * Unnamed Lisp interpreter (WIP)
 *
 * The goal of this Lisp interpreter is not to be small or fast, but to be simple and readable without sacrificing important optimizations.
 *
 * In terms of features, this implementation only aims to provide the bare minimum to meaningfully support uKanren.
 * (It might be there now, but it's hard to say - porting that over will be another learning project down the line.)
 *
 * Certain design decisions are inspired by Justine Tunney's SectorLISP and Dr. Robert van Engelen's tinylisp.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

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


// **************** Top-level definitions ****************

// X macro: Built-in symbols for which a primitive will be defined
#define FOREACH_PRIM(X) \
	X("\004cons", l_cons) \
	X("\003car", l_car) \
	X("\003cdr", l_cdr) \
	X("\005atom?", l_atom) \
	X("\003eq?", l_eq) \
	X("\005null?", l_null) \
	X("\004eval", l_eval) \
	X("\005quote", l_quote) \
	X("\004cond", l_cond) \
	X("\006lambda", l_lambda) \
	X("\003let", l_let) \
	X("\005macro", l_macro)

// X macro: All built-in symbols (with or without a corresponding primitive)
#define FOREACH_SYMVAR(X) \
	X("\002#t", l_t) \
	X("\006define", l_define) \
	FOREACH_PRIM(X)

// Declare a function for each primitive
#define DECLARE_FUNC(SYM,ID) void *ID(void *args, void **cont, void **envp);
FOREACH_PRIM(DECLARE_FUNC)
typedef void *(*l_prim_t)(void *args, void **cont, void **envp); // Function pointer type for Lisp primitives

// Declare character pointer variables for each built-in symbol
#define DECLARE_SYMVAR(SYM,ID) char *ID##_sym;
FOREACH_SYMVAR(DECLARE_SYMVAR)

// Declare a compile-time numeric index for each primitive
enum l_prim_e {
#define DEFINE_ENUM_VAL(SYM,ID) ID##_e,
	FOREACH_PRIM(DEFINE_ENUM_VAL)
	NUM_PRIMS
};

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
#define IN(X,T) ((uintptr_t)(X) >= (uintptr_t)(T) \
		&& (uintptr_t)(X) < (uintptr_t)(T) + sizeof(T))


// **************** Basic value operations ****************

void *cons(void *x, void *y)
{
	void **car = next_cell++;
	void **cdr = next_cell++;
	*car = x;
	*cdr = y;
	return car;
}

#define list1(a) cons(a, NULL)
#define list2(a, b) cons(a, list1(b))
#define list3(a, b, c) cons(a, list2(b, c))
#define list4(a, b, c, d) cons(a, list3(b, c, d))

#define CAR(l) ((void **)l)
void *car(void *l)
{
	if (!IN(l, cells))
		return ERROR;
	return *CAR(l);
}

#define CDR(l) ((void **)l + 1)
void *cdr(void *l)
{
	if (!IN(l, cells))
		return ERROR;
	return *CDR(l);
}

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
	if (peek == ';') { // Comments
		while (peek != '\n')
			next();
		return read();
	} else if (peek == '\'') { // Quoted expression
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

void **pre_eval = cells;

void *copy(void *x, ptrdiff_t cell_offset)
{
	// Copy an object, offsetting all cell pointers above pre_eval
	if (!IN(x, cells) || (void **)x < pre_eval) // No need to copy values below the pre-eval point
		return x;
	void *a = copy(car(x), cell_offset);
	void *d = copy(cdr(x), cell_offset);
	return (void **)cons(a, d) + cell_offset;
}

void gc(void **ret, void **env)
{
	// Copying garbage collection for a return value and the environment
	if (next_cell == pre_eval) // Ellide useless calls
		return;
#ifndef DISABLE_GC
	// Copy the return value and environment as needed, offsetting cells to match their post-GC position
	void **pre_copy = next_cell;
	ptrdiff_t diff = pre_copy - pre_eval;
	void *post_gc_env = copy(*env, -diff);
	void *post_gc_ret = copy(*ret, -diff);
	// Move the copied cells to the pre-eval position
	ptrdiff_t copy_size = next_cell - pre_copy;
	memcpy(pre_eval, pre_copy, copy_size * sizeof(*pre_copy));
	// Correct next_cell to account for GC
	next_cell = pre_eval + copy_size;
	*env = post_gc_env;
	*ret = post_gc_ret;
	DEBUG(printf("Cells used: %ld -> %ld\n", pre_copy - cells, next_cell - cells);)
#else
	DEBUG(printf("Cells used: %ld\n", next_cell - cells);)
#endif
}

void *define(void *k, void *v, void *env)
{
	// Update an existing binding for k from this eval call to point to v
	for (void *e = env; e > (void *)pre_eval; e = cdr(e)) {
		if (caar(e) == k) {
			*CDR(car(e)) = v;
			return env;
		}
	}
	// If not possible, make a new binding
	return cons(cons(k, v), env);
}


/* **************** Interpreter (modified for TCO) **************** */

// The implementation of TCO here aims to avoid deviating too heavily from the original interpreter structure
// The basic idea here is to:
// - Add a new eval function that relies on an infinite loop to step through the evaluation
// - Modify the old interpreter functions to return an expression to continue from instead of calling eval, where possible
// The identifiers cont, envp, and INCOMPLETE signal where these modifications happened.

void *eval(void *x, void *env);

// Global Lisp environment (populated at runtime)
void *defines = NULL;

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
		return car(l);
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
		return pairlis(cdr(ks), cdr(vs), define(car(ks), car(vs), env));
	else
		return define(ks, vs, env); // Bind remaining values to dangling key
}

void *apply(void *f, void *args, void **cont, void **envp)
{
	// Apply non-primitive function f to unevaluated args in environment env (modified for TCO)
	if (!IN(f, cells))
		return ERROR;

	// Closures are structured as (lambda/macro args body env)
	// `env` is nil if there is no meaningful environment to close over
	// This signals for us to reference the global environment instead, permitting recursive function definitions
	// (Credit goes entirely to tinylisp for this idea)

	void *env = cadddr(f);
	if (!env)
		env = defines;

	if (car(f) == l_macro_sym) { // macro -> don't eval args, but do eval result before continuing
		*cont = eval(caddr(f), pairlis(cadr(f), args, env));
		return INCOMPLETE;
	} else if (car(f) == l_lambda_sym) { // lambda -> eval args, continue with body
		*envp = pairlis(cadr(f), evlis(args, *envp), env);
		*cont = caddr(f);
		return INCOMPLETE;
	}
	return ERROR;
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
	if (IN(x, syms)) { // Symbol -> return variable binding
		return cdr(assoc(x, env));
	} else if (IN(x, cells)) { // List -> interpret S-expression
		void *f = eval(car(x), env);
		if (IN(f, prims)) // Primitive -> call C function
			return (*(l_prim_t *)f)(cdr(x), cont, envp);
		else // Non-primitive -> apply function or macro
			return apply(f, cdr(x), cont, envp);
	}
	// Nil or unknown -> return as-is
	return x;
}

void *eval(void *x, void *env)
{
	// Tail-call optimized eval
	DEBUG(static int level = 0; level++;)
	DEBUG(printf("%*sL%d eval: ", 4*level, "", level); print(x); printf("\n");)
	void **old_pre_eval = pre_eval;
	pre_eval = next_cell;

	void *ret;
	for (;;) {
		ret = eval_step(&x, &env);
		if (ret != INCOMPLETE)
			break;
		gc(&x, &env); // Collect garbage, keeping the continuation expression
		// ^ TODO: This call causes inconsistency with eq? due to deep copying env
		// Need to determine whether this is the only problematic call and find a good compromise

#ifdef DISABLE_TCO
		// If TCO is disabled for testing purposes, eval recursively
		ret = eval(x, env);
		break;
#endif
		DEBUG(printf("%*sL%d step: ", 4*level, "", level); print(x); printf("\n");)
	}
	DEBUG(printf("%*sL%d result: ", 4*level, "", level); print(ret); printf("\n");)
	DEBUG(level--;)

	gc(&ret, &env); // Collect garbage, keeping the return value
	pre_eval = old_pre_eval;
	return ret;
}


// **************** REPL ****************

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
	for (;;) {
		void *nil = NULL;
		void *exp = read();
		if (car(exp) == l_define_sym) {
			// Handle defines
			if (IN(cadr(exp), syms))
				defines = define(cadr(exp), eval(caddr(exp), defines), defines);
		} else {
			// Evaluate expressions
			print(eval(exp, defines));
			printf("\n");
		}
		gc(&nil, &defines); // Destroy return value and keep definitions
	}
}


// **************** Primitives ****************

// "Special forms" - i.e., primitives which DO NOT evaluate their arguments

void *l_quote(void *args, void **cont, void **envp)
{
	(void)envp;
	(void)cont; // no TCO
	return car(args);
}

void *l_cond(void *args, void **cont, void **envp)
{
	*cont = evcon(args, *envp);
	return INCOMPLETE;
}

void *l_lambda(void *args, void **cont, void **envp)
{
	(void)cont; // no TCO
	return list4(l_lambda_sym, car(args), cadr(args), *envp == defines ? NULL : *envp);
}

void *l_let(void *args, void **cont, void **envp)
{
	*cont = evlet(car(args), cadr(args), envp);
	return INCOMPLETE;
}

void *l_macro(void *args, void **cont, void **envp)
{
	(void)cont; // no TCO
	return list4(l_macro_sym, car(args), cadr(args), *envp == defines ? NULL : *envp);
}

// "Functions" - i.e., primitives which DO evaluate their arguments

void *l_cons(void *args, void **cont, void **envp)
{
	(void)cont; // no TCO
	args = evlis(args, *envp); // evaluate args
	return cons(car(args), cadr(args));
}

void *l_car(void *args, void **cont, void **envp)
{
	(void)cont; // no TCO
	args = evlis(args, *envp); // evaluate args
	return caar(args);
}

void *l_cdr(void *args, void **cont, void **envp)
{
	(void)cont; // no TCO
	args = evlis(args, *envp); // evaluate args
	return cdar(args);
}

void *l_atom(void *args, void **cont, void **envp)
{
	(void)cont; // no TCO
	args = evlis(args, *envp); // evaluate args
	if (IN(car(args), cells))
		return NULL;
	return l_t_sym;
}

void *l_eq(void *args, void **cont, void **envp)
{
	(void)cont; // no TCO
	args = evlis(args, *envp); // evaluate args
	if (car(args) == cadr(args))
		return l_t_sym;
	return NULL;
}

void *l_null(void *args, void **cont, void **envp)
{
	(void)cont; // no TCO
	args = evlis(args, *envp); // evaluate args
	if (car(args))
		return NULL;
	return l_t_sym;
}

void *l_eval(void *args, void **cont, void **envp)
{
	args = evlis(args, *envp); // evaluate args
	*cont = car(args);
	return INCOMPLETE;
}
