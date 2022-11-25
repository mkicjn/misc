#include <stdio.h>
#include <stdint.h>

#define DOLIT(x) *(++sp) = x;

register intptr_t *sp asm ("rbx");

void inc()
{
	(*sp)++; // 1+
}

void dec()
{
	(*sp)--; // 1-
}

void div2()
{
	(*sp) >>= 1; // 2/
}

void mul2()
{
	(*sp) <<= 1; // 2*
}

void add()
{
	sp--; // +
	sp[0] += sp[1];
}

void drop()
{
	sp--; // DROP
}

void dup()
{
	sp[1] = sp[0]; // DUP
	sp++;
}

void swap()
{
	intptr_t wr1;
	wr1 = sp[0]; // SWAP
	sp[0] = sp[-1];
	sp[-1] = wr1;
}

void rot()
{
	intptr_t wr1, wr2;
	wr1 = sp[-1]; // ROT
	wr2 = sp[0];
	sp[0] = sp[-2];
	sp[-2] = wr1;
	sp[-1] = wr2;
}

void max()
{
	intptr_t wr1, wr2;
	wr1 = sp[0]; // MAX
	wr2 = sp[-1];
	sp[-1] = wr1 > wr2 ? wr1 : wr2;
	sp--;
}

void zeq()
{
	sp[0] = (sp[0] == 0); // 0=
}

void and()
{
	sp--; // AND
	sp[0] &= sp[1];
}

void gt()
{
	sp--; // >
	sp[0] = (sp[0] > sp[1]);
}

void dot()
{
	printf("%ld ", *(sp--)); // .
}

void cr()
{
	putchar('\n');
}


// : ITER  DUP 1 AND 0= IF  2/  ELSE  DUP 2* + 1+  THEN ;
// : COLLATZ  0 SWAP  BEGIN  DUP 1 >  WHILE  SWAP 1+ SWAP  ITER  REPEAT  DROP ;
// : MAXLEN  0 SWAP  BEGIN   DUP COLLATZ ROT MAX SWAP  1- DUP 0= UNTIL DROP ;

void iter()
{
	dup();
	DOLIT(1);
	and();
	zeq();
	if (*(sp--)) { // IF
		div2();
	} else { // ELSE
		dup();
		mul2();
		add();
		inc();
	} // THEN
};

void collatz()
{
	DOLIT(0);
	swap();
	for (;;) { // BEGIN
		dup();
		DOLIT(1);
		gt();
	if (!*(sp--)) break; // WHILE
		swap();
		inc();
		swap();
		iter();
	} // REPEAT
	drop();
};

void maxlen()
{
	DOLIT(0);
	swap();
	do { // BEGIN
		dup();
		collatz();
		rot();
		max();
		swap();
		dec();
		dup();
		zeq();
	} while (!*(sp--)); // AGAIN
	drop();
};

void program()
{
	DOLIT(1000000);
	maxlen();
	dot();
};

int main()
{
	intptr_t stack[1024];

	sp = stack;

	program();
	return *(sp--);
}
