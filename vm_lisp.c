//`which tcc` $CFLAGS -run $0 $@; exit $?

// (Stack-based Lisp VM idea - dormant)
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define ERROR ((void *)1)


// Symbols / String Interning

#define MAX_SYMS 100000
char sym_space[MAX_SYMS];
char *free_sym = &sym_space[0];

static inline bool symbolp(char *x)
{
	return &sym_space[0] <= x && x < &sym_space[MAX_SYMS];
}

char *intern(char *cstr)
{
	size_t len = strlen(cstr);
	if (len > 255)
		len = 255;

	for (char *sym = &sym_space[0]; sym < free_sym; sym += sym[0] + 1) {
		if (sym[0] == len && memcmp(sym + 1, cstr, len) == 0)
			return sym;
	}

	*(free_sym++) = len;
	memcpy(free_sym, cstr, len);
	free_sym += len;
	return free_sym - (len + 1);
}


// Cells / List Operations

#define MAX_CELLS 100000
void *cell_space[MAX_CELLS];
void **free_cells = &cell_space[0];

static inline bool listp(void **x)
{
	return &cell_space[0] <= x && x < &cell_space[MAX_CELLS];
}

static inline void *cons(void *a, void *d)
{
	void **c = free_cells;
	free_cells += 2;
	c[0] = a;
	c[1] = d;
	return c;
}

static inline void *car(void **x)
{
	return listp(x) ? x[0] : ERROR;
}

static inline void *cdr(void **x)
{
	return listp(x) ? x[1] : ERROR;
}


// Stack / Stack operations

#define MAX_STACK 1000
void *stack_space[MAX_STACK];
void **stack = &stack_space[MAX_STACK];

#define MAX_FRAMES 100
void **frame_space[MAX_FRAMES];
void ***frames = &frame_space[MAX_FRAMES];

#define PUSH(x) (--x)
#define  POP(x) (x++)

void enter(void)
{
	*PUSH(frames) = stack;
}

void *arg(int n)
{
	return frames[0][n];
}

void leave(void)
{
	void *ret = *POP(stack);
	stack = *POP(frames);
	*PUSH(stack) = ret;
}


// Lisp-to-Stack-Machine Bindings

void st_car(void)
{
	stack[0] = car(stack[0]);
}

void st_cdr(void)
{
	stack[0] = cdr(stack[0]);
}

void st_cons(void)
{
	void *c = cons(stack[1], stack[0]);
	POP(stack);
	stack[0] = c;
}


// Main (test program)

// TODO: Is putting arguments on a stack going to be amenable to TCO?
// Seems to be potentially a fatal flaw of this idea...

int main()
{
	char *sym_t = intern("t");
	char *sym_nil = intern("nil");
	char *sym_nil2 = intern("nil");
	printf("%.*s\n", sym_t[0], sym_t+1);
	printf("%.*s\n", sym_nil[0], sym_nil+1);
	printf("%.*s\n", sym_nil2[0], sym_nil2+1);
	printf("%p ... %p\n", sym_nil, sym_nil2);
	return 0;
}
