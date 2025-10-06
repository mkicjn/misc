// (very WIP)

// Cell / List operations

#define MAX_CELLS 100000
void *cell_space[MAX_CELLS];
void **cells = &cell_space[0];

void *cons(void *a, void *d)
{
	void **c = cells;
	cells += 2;
	c[0] = a;
	c[1] = d;
	return c;
}

void *car(void **x)
{
	return x[0];
}

void *cdr(void **x)
{
	return x[1];
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


// Main (Test program)

struct vm_regs {
	void **cells;
	void **stack;
	void ***frames;
};
// ^ ???

int main()
{
	return 0;
}
