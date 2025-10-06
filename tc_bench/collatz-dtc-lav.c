#include <stdio.h>
#include <stdint.h>

// FIXME: This implementation may be at a disadvantage because it lacks a TOS register

int main()
{
	// : ITER  DUP 1 AND 0= IF  2/  ELSE  DUP 2* + 1+  THEN ;
	// : COLLATZ  0 SWAP  BEGIN  DUP 1 >  WHILE  SWAP 1+ SWAP  ITER  REPEAT  DROP ;
	// : MAXLEN  0 SWAP  BEGIN   DUP COLLATZ ROT MAX SWAP  1- DUP 0= UNTIL DROP ;

	static void *iter[] = {
		/* 0 */ &&dup,
		/* 1 */ &&dolit, (void *)1,
		/* 3 */ &&and,
		/* 4 */ &&zeq,
		/* 5 */ &&jz, (void *)&iter[10],
		/* 7 */ &&div2,
		/* 8 */ &&jmp, (void *)&iter[14],
		/* 10 */ &&dup,
		/* 11 */ &&mul2,
		/* 12 */ &&add,
		/* 13 */ &&inc,
		/* 14 */ &&exit
	};

	static void *collatz[] = {
		/* 0 */ &&dolit, (void *)0,
		/* 2 */ &&swap,
		/* 3 */ &&dup,
		/* 4 */ &&dolit, (void *)1,
		/* 6 */ &&gt,
		/* 7 */ &&jz, (void *)&collatz[16],
		/* 9 */ &&swap,
		/* 10 */ &&inc,
		/* 11 */ &&swap,
		/* 12 */ &&docol, (void *)iter,
		/* 14 */ &&jmp, (void *)&collatz[3],
		/* 16 */ &&drop,
		/* 17 */ &&exit
	};

	static void *maxlen[] = {
		/* 0 */ &&dolit, (void *)0,
		/* 2 */ &&swap,
		/* 3 */ &&dup,
		/* 4 */ &&docol, (void *)collatz,
		/* 6 */ &&rot,
		/* 7 */ &&max,
		/* 8 */ &&swap,
		/* 9 */ &&dec,
		/* 10 */ &&dup,
		/* 11 */ &&zeq,
		/* 12 */ &&jz, (void *)&maxlen[3],
		/* 14 */ &&drop,
		/* 15 */ &&exit
	};

	static void *program[] = {
		/* 0 */ &&dolit, (void *)1000000,
		/* 2 */ &&docol, (void *)&maxlen,
		/* 4 */ &&dot,
		/* 5 */ &&bye
	};

	intptr_t stack[1024];
	void **return_stack[1024];

	void **ip = program;
	intptr_t *sp = stack;
	void ***rp = return_stack;
	intptr_t wr1 = 0, wr2 = 0;

next: asm("next:");
	goto **(ip++);

docol: asm("docol:");
	*(++rp) = ip+1; // DOCOL
	ip = *ip;
	goto **(ip++);

exit: asm("exit:");
	ip = *(rp--); // EXIT
	goto **(ip++);


dolit: asm("dolit:");
	*(++sp) = *(intptr_t *)(ip++); // DOLIT
	goto **(ip++);

inc: asm("inc:");
	(*sp)++; // 1+
	goto **(ip++);

dec: asm("dec:");
	(*sp)--; // 1-
	goto **(ip++);

div2: asm("div2:");
	(*sp) >>= 1; // 2/
	goto **(ip++);

mul2: asm("mul2:");
	(*sp) <<= 1; // 2*
	goto **(ip++);

add: asm("add:");
	sp--; // +
	sp[0] += sp[1];
	goto **(ip++);

drop: asm("drop:");
	sp--; // DROP
	goto **(ip++);

dup: asm("dup:");
	sp[1] = sp[0]; // DUP
	sp++;
	goto **(ip++);

swap: asm("swap:");
	wr1 = sp[0]; // SWAP
	sp[0] = sp[-1];
	sp[-1] = wr1;
	goto **(ip++);

rot: asm("rot:");
	wr1 = sp[-1]; // ROT
	wr2 = sp[0];
	sp[0] = sp[-2];
	sp[-2] = wr1;
	sp[-1] = wr2;
	goto **(ip++);

max: asm("max:");
	wr1 = sp[0]; // MAX
	wr2 = sp[-1];
	sp[-1] = wr1 > wr2 ? wr1 : wr2;
	sp--;
	goto **(ip++);

jmp: asm("jmp:");
	ip = *ip; // BRANCH
	goto **(ip++);

jz: asm("jz:");
	wr1 = *(sp--); // ?BRANCH
	if (!wr1)
		ip = *ip;
	else
		ip++;
	goto **(ip++);

zeq: asm("zeq:");
	sp[0] = (sp[0] == 0); // 0=
	goto **(ip++);

and: asm("and:");
	sp--; // AND
	sp[0] &= sp[1];
	goto **(ip++);

gt: asm("gt:");
	sp--; // >
	sp[0] = (sp[0] > sp[1]);
	goto **(ip++);

dot: asm("dot:");
	printf("%ld ", *(sp--)); // .
	goto **(ip++);

cr: asm("cr:");
	putchar('\n');
	goto **(ip++);

bye: asm("bye:");
	return *sp; // BYE

}
