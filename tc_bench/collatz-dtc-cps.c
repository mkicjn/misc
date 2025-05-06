#include "../cpp_dict.c"

int main()
{
	// : ITER  DUP 1 AND 0= IF  2/  ELSE  DUP 2* + 1+  THEN ;
	// : COLLATZ  0 SWAP  BEGIN  DUP 1 >  WHILE  SWAP 1+ SWAP  ITER  REPEAT  DROP ;
	// : MAXLEN  0 SWAP  BEGIN   DUP COLLATZ ROT MAX SWAP  1- DUP 0= UNTIL DROP ;

	static void (*iter[])() = {
		/* 0 */ dup_code,
		/* 1 */ dolit_code, (void *)1,
		/* 3 */ and_code,
		/* 4 */ zeq_code,
		/* 5 */ jz_code, (void *)&iter[10],
		/* 7 */ div2_code,
		/* 8 */ jmp_code, (void *)&iter[14],
		/* 10 */ dup_code,
		/* 11 */ mul2_code,
		/* 12 */ add_code,
		/* 13 */ inc_code,
		/* 14 */ exit_code
	};

	static void (*collatz[])() = {
		/* 0 */ dolit_code, (void *)0,
		/* 2 */ swap_code,
		/* 3 */ dup_code,
		/* 4 */ dolit_code, (void *)1,
		/* 6 */ gt_code,
		/* 7 */ jz_code, (void *)&collatz[16],
		/* 9 */ swap_code,
		/* 10 */ inc_code,
		/* 11 */ swap_code,
		/* 12 */ docol_code, (void *)iter,
		/* 14 */ jmp_code, (void *)&collatz[3],
		/* 16 */ drop_code,
		/* 17 */ exit_code
	};

	static void (*maxlen[])() = {
		/* 0 */ dolit_code, (void *)0,
		/* 2 */ swap_code,
		/* 3 */ dup_code,
		/* 4 */ docol_code, (void *)collatz,
		/* 6 */ rot_code,
		/* 7 */ max_code,
		/* 8 */ swap_code,
		/* 9 */ dec_code,
		/* 10 */ dup_code,
		/* 11 */ zeq_code,
		/* 12 */ jz_code, (void *)&maxlen[3],
		/* 14 */ drop_code,
		/* 15 */ exit_code
	};

	static void (*prog[])() = {
		dolit_code, (void *)1000000,
		docol_code, (void *)maxlen,
		dot_code,
		bye_code
	};


	static intptr_t stack[1000];
	static intptr_t rstack[1000];
	intptr_t *dp = NULL;
	dtc_t *ip = prog;
	intptr_t *sp = stack;
	intptr_t *rp = rstack;
	intptr_t tos = 0;

	NEXT();

	return 0;
}
