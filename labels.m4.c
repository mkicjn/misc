#include <termios.h>
#include <stdio.h>
typedef long cell_t;

typedef struct link_s {
	struct link_s *prev;
	char *name;
	cell_t len;
} link_t;

void toggle_raw(void)
{
	struct termios t;
	tcgetattr(1,&t);
	t.c_lflag^=ICANON|ECHO;
	tcsetattr(1,TCSANOW,&t);
}

int main(int argc,char **argv)
{
	(void) argc; (void) argv;
m4_dnl/*
m4_define(`m4_prev',`0')m4_dnl
m4_define(`m4_prim',`m4_divert(0)
	typedef struct {link_t link; void *xt[1];} $1_t;
	static $1_t $1_def={{m4_prev,"$2",m4_len($2)},{&&$1_code}}; (void) $1_def;m4_dnl
m4_define(`m4_prev',&$1_def.link)m4_dnl
m4_divert(1)m4_dnl
$1_code:')m4_dnl*/
m4_divert(1)

	static void **inc[]={&&docol_code,dolit_def.xt,(void *)1,add_def.xt,exit_def.xt};
	static void **test[]={key_def.xt,emit_def.xt,dolit_def.xt,(void *)2,(void *)&inc,bye_def.xt};

#define STACK_SIZE 100
#define RSTACK_SIZE 100
	cell_t stack[STACK_SIZE];
	cell_t rstack[RSTACK_SIZE];

	register void ***ip=test;
	register union {void **p; cell_t c;} w;
	register cell_t *sp=stack;
	register cell_t *rp=rstack;
	register cell_t tos=0;
#define PUSH(x) *(x++)
#define POP(x) *(--x)

	toggle_raw();
next:
	goto **(w.p=*(ip++));

m4_prim(bye,BYE)
	toggle_raw();
	return tos;
m4_prim(dolit,DOLIT)
	PUSH(sp)=tos;
	tos=(cell_t)*(ip++);
	goto next;
m4_prim(docol,DOCOL)
	PUSH(rp)=(cell_t)ip;
	ip=(void ***)w.p+1;
	goto next;
m4_prim(exit,EXIT)
	ip=(void ***)POP(rp);
	goto next;

m4_prim(dup,DUP)
	PUSH(sp)=tos;
	goto next;
m4_prim(drop,DROP)
	tos=POP(sp);
	goto next;
m4_prim(swap,SWAP)
	w.c=sp[-1];
	sp[-1]=tos;
	tos=w.c;
	goto next;
m4_prim(rot,ROT)
	w.c=tos;
	tos=sp[-1];
	sp[-1]=sp[-2];
	sp[-2]=w.c;
	goto next;

m4_prim(key,KEY)
	PUSH(sp)=tos;
	tos=getchar();
	goto next;
m4_prim(emit,EMIT)
	putchar(tos);
	tos=POP(sp);
	goto next;


m4_dnl/*
m4_define(`m4_2op',`m4_dnl
m4_prim($1,$2)
	tos=(tos$3POP(sp));
	goto next;
')m4_dnl
m4_define(`m4_1op',`m4_dnl
m4_prim($1,$2)
	tos=($3tos$4);
	goto next;
')m4_dnl*/
m4_2op(add,+,+) m4_2op(sub,-,-)
m4_2op(mul,*,*) m4_2op(div,/,/)
m4_2op(lsh,LSHIFT,<<) m4_2op(rsh,RSHIFT,>>)
m4_1op(mul2,2*,,*2) m4_1op(div2,2/,,/2)
m4_2op(and,AND,&) m4_2op(or,OR,|) m4_2op(xor,XOR,^)
m4_1op(not,INVERSE,~) m4_1op(neg,NEGATE,-)

m4_2op(gt,>,>) m4_1op(gtz,0>,,>0)
m4_2op(gte,>=,>=) m4_1op(gtez,0>=,,>=0)
m4_2op(lt,<,<) m4_1op(ltz,0<,,<0)
m4_2op(lte,<=,<=) m4_1op(ltez,0<=,,<=0)
m4_2op(eq,=,==) m4_1op(eqz,0=,,==0)
m4_2op(neq,<>,!=) m4_1op(neqz,0<>,,!=0)

}
