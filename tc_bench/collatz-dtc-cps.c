#include "../cpp_dict.c"

int main()
{
	// : ITER  DUP 1 AND 0= IF  2/  ELSE  DUP 2* + 1+  THEN ;
	// : COLLATZ  0 SWAP  BEGIN  DUP 1 >  WHILE  SWAP 1+ SWAP  ITER  REPEAT  DROP ;
	// : MAXLEN  0 SWAP  BEGIN   DUP COLLATZ ROT MAX SWAP  1- DUP 0= UNTIL DROP ;

	static intptr_t iter[] = {
		/* 0 */ (intptr_t)dup_code,
		/* 1 */ (intptr_t)dolit_code, 1,
		/* 3 */ (intptr_t)and_code,
		/* 4 */ (intptr_t)zeq_code,
		/* 5 */ (intptr_t)jz_code, (intptr_t)&iter[10],
		/* 7 */ (intptr_t)div2_code,
		/* 8 */ (intptr_t)jmp_code, (intptr_t)&iter[14],
		/* 10 */ (intptr_t)dup_code,
		/* 11 */ (intptr_t)mul2_code,
		/* 12 */ (intptr_t)add_code,
		/* 13 */ (intptr_t)inc_code,
		/* 14 */ (intptr_t)exit_code
	};

	static intptr_t collatz[] = {
		/* 0 */ (intptr_t)dolit_code, 0,
		/* 2 */ (intptr_t)swap_code,
		/* 3 */ (intptr_t)dup_code,
		/* 4 */ (intptr_t)dolit_code, 1,
		/* 6 */ (intptr_t)gt_code,
		/* 7 */ (intptr_t)jz_code, (intptr_t)&collatz[16],
		/* 9 */ (intptr_t)swap_code,
		/* 10 */ (intptr_t)inc_code,
		/* 11 */ (intptr_t)swap_code,
		/* 12 */ (intptr_t)docol_code, (intptr_t)iter,
		/* 14 */ (intptr_t)jmp_code, (intptr_t)&collatz[3],
		/* 16 */ (intptr_t)drop_code,
		/* 17 */ (intptr_t)exit_code
	};

	static intptr_t maxlen[] = {
		/* 0 */ (intptr_t)dolit_code, 0,
		/* 2 */ (intptr_t)swap_code,
		/* 3 */ (intptr_t)dup_code,
		/* 4 */ (intptr_t)docol_code, (intptr_t)collatz,
		/* 6 */ (intptr_t)rot_code,
		/* 7 */ (intptr_t)max_code,
		/* 8 */ (intptr_t)swap_code,
		/* 9 */ (intptr_t)dec_code,
		/* 10 */ (intptr_t)dup_code,
		/* 11 */ (intptr_t)zeq_code,
		/* 12 */ (intptr_t)jz_code, (intptr_t)&maxlen[3],
		/* 14 */ (intptr_t)drop_code,
		/* 15 */ (intptr_t)exit_code
	};

	static intptr_t prog[] = {
		(intptr_t)dolit_code, 1000000,
		(intptr_t)docol_code, (intptr_t)maxlen,
		(intptr_t)dot_code,
		(intptr_t)bye_code
	};


	static intptr_t stack[1000];
	static intptr_t rstack[1000];
	intptr_t *dp = NULL;
	intptr_t *ip = prog;
	intptr_t *sp = stack;
	intptr_t *rp = rstack;
	intptr_t tos = 0;

	NEXT();

	return 0;
}
