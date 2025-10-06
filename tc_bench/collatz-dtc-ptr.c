// Direct threaded code
// VM as struct representation

#include <stdio.h>
#include <stdint.h>

struct vm_regs {
	intptr_t *dp;// = NULL;
	intptr_t *ip;// = prog;
	intptr_t *sp;// = stack;
	intptr_t *rp;// = rstack;
	intptr_t tos;// = 0;
};

typedef void (*vm_fn_t)(struct vm_regs *);

#define PUSH(sp) (++sp)
#define POP(sp) (sp--)

// Word definitions

void key(struct vm_regs *vm)
{
	*PUSH(vm->sp) = vm->tos;
	vm->tos = getchar();
}

void emit(struct vm_regs *vm)
{
	putchar(vm->tos);
	vm->tos = *POP(vm->sp);
}

void dot(struct vm_regs *vm)
{
	printf("%ld\n", vm->tos);
	vm->tos = *POP(vm->sp);
}

void enter(struct vm_regs *vm)
{
	*PUSH(vm->rp) = (intptr_t)(&vm->ip[1]);
	vm->ip = (intptr_t *)vm->ip[0];
}

void leave(struct vm_regs *vm)
{
	vm->ip = (intptr_t *)(*POP(vm->rp));
}

void dolit(struct vm_regs *vm)
{
	*PUSH(vm->sp) = vm->tos;
	vm->tos = vm->ip[0];
	vm->ip++;
}

void dup(struct vm_regs *vm)
{
	*PUSH(vm->sp) = vm->tos;
}

void drop(struct vm_regs *vm)
{
	vm->tos = *POP(vm->sp);
}

void swap(struct vm_regs *vm)
{
	intptr_t a = vm->tos;
	intptr_t b = vm->sp[0];
	vm->tos = b;
	vm->sp[0] = a;
}

#define WORD_2OP(name, op) \
void name(struct vm_regs *vm) \
{ \
	vm->tos = (*POP(vm->sp) op vm->tos); \
}
WORD_2OP(add, +)
WORD_2OP(sub, -)
WORD_2OP(lsh, <<)
WORD_2OP(rsh, >>)
WORD_2OP(gt, >)
WORD_2OP(and, &)

void mul2(struct vm_regs *vm)
{
	vm->tos <<= 1;
}

void div2(struct vm_regs *vm)
{
	vm->tos >>= 1;
}

void inc(struct vm_regs *vm)
{
	vm->tos++;
}

void dec(struct vm_regs *vm)
{
	vm->tos--;
}

void max(struct vm_regs *vm)
{
	intptr_t x = *POP(vm->sp);
	if (x > vm->tos)
		vm->tos = x;
}

void rot(struct vm_regs *vm)
{
	intptr_t a = vm->tos;
	intptr_t b = vm->sp[0];
	intptr_t c = vm->sp[-1];
	vm->tos = c;
	vm->sp[0] = a;
	vm->sp[-1] = b;
}

void jmp(struct vm_regs *vm)
{
	vm->ip = (intptr_t *)vm->ip[0];
}

void jz(struct vm_regs *vm)
{
	if (vm->tos == 0) {
		vm->ip = (intptr_t *)vm->ip[0];
	} else {
		vm->ip++;
	}
	vm->tos = *POP(vm->sp);
}

void zeq(struct vm_regs *vm)
{
	vm->tos = (vm->tos == 0);
}


int main()
{
	// : ITER  DUP 1 AND 0= IF  2/  ELSE  DUP 2* + 1+  THEN ;
	// : COLLATZ  0 SWAP  BEGIN  DUP 1 >  WHILE  SWAP 1+ SWAP  ITER  REPEAT  DROP ;
	// : MAXLEN  0 SWAP  BEGIN   DUP COLLATZ ROT MAX SWAP  1- DUP 0= UNTIL DROP ;

	static intptr_t iter[] = {
		/* 0 */ (intptr_t)&dup,
		/* 1 */ (intptr_t)&dolit, 1,
		/* 3 */ (intptr_t)&and,
		/* 4 */ (intptr_t)&zeq,
		/* 5 */ (intptr_t)&jz, (intptr_t)&iter[10],
		/* 7 */ (intptr_t)&div2,
		/* 8 */ (intptr_t)&jmp, (intptr_t)&iter[14],
		/* 10 */ (intptr_t)&dup,
		/* 11 */ (intptr_t)&mul2,
		/* 12 */ (intptr_t)&add,
		/* 13 */ (intptr_t)&inc,
		/* 14 */ (intptr_t)&leave
	};

	static intptr_t collatz[] = {
		/* 0 */ (intptr_t)&dolit, 0,
		/* 2 */ (intptr_t)&swap,
		/* 3 */ (intptr_t)&dup,
		/* 4 */ (intptr_t)&dolit, 1,
		/* 6 */ (intptr_t)&gt,
		/* 7 */ (intptr_t)&jz, (intptr_t)&collatz[16],
		/* 9 */ (intptr_t)&swap,
		/* 10 */ (intptr_t)&inc,
		/* 11 */ (intptr_t)&swap,
		/* 12 */ (intptr_t)&enter, (intptr_t)iter,
		/* 14 */ (intptr_t)&jmp, (intptr_t)&collatz[3],
		/* 16 */ (intptr_t)&drop,
		/* 17 */ (intptr_t)&leave
	};

	static intptr_t maxlen[] = {
		/* 0 */ (intptr_t)&dolit, 0,
		/* 2 */ (intptr_t)&swap,
		/* 3 */ (intptr_t)&dup,
		/* 4 */ (intptr_t)&enter, (intptr_t)collatz,
		/* 6 */ (intptr_t)&rot,
		/* 7 */ (intptr_t)&max,
		/* 8 */ (intptr_t)&swap,
		/* 9 */ (intptr_t)&dec,
		/* 10 */ (intptr_t)&dup,
		/* 11 */ (intptr_t)&zeq,
		/* 12 */ (intptr_t)&jz, (intptr_t)&maxlen[3],
		/* 14 */ (intptr_t)&drop,
		/* 15 */ (intptr_t)&leave
	};

	static intptr_t prog[] = {
		(intptr_t)&dolit, 1000000,
		(intptr_t)&enter, (intptr_t)maxlen,
		(intptr_t)&dot,
		(intptr_t)0
	};


	static intptr_t stack[1000];
	static intptr_t rstack[1000];

	struct vm_regs vm;
	vm.dp = NULL;
	vm.ip = prog;
	vm.sp = stack;
	vm.rp = rstack;
	vm.tos = 0;

	while (*vm.ip != 0) {
		vm_fn_t f = (vm_fn_t)(*(vm.ip++));
		f(&vm);
	}

	return 0;
}
