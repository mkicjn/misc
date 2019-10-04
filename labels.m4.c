typedef long cell_t;

typedef struct link_s {
	struct link_s *prev;
	char *name;
	cell_t len;
} link_t;

int main(int argc,char **argv)
{ (void) argc; (void) argv;
m4_dnl/*
m4_define(`m4_prim',`m4_divert(0)
	typedef struct {link_t link; void *xt[1];} $1_t;
	static $1_t $1_def={{0,"$2",m4_len($2)},{&&$1_code}}; (void) $1_def;m4_dnl
m4_divert(1)m4_dnl
$1_code:')m4_dnl*/
m4_divert(1)

	static void **inc[]={&&enter_code,dolit_def.xt,(void *)1,add_def.xt,exit_def.xt};
	static void **test[]={dolit_def.xt,(void *)2,(void *)&inc,bye_def.xt};
	cell_t stack[100];
	cell_t rstack[100];

	register void ***ip=test;
	register void **wp=0;
	register cell_t *sp=stack;
	register cell_t *rp=rstack;
	register cell_t tos=0;

next:
	wp=*(ip++);
	goto **wp;

m4_prim(bye,BYE)
	return tos;

m4_prim(dolit,DOLIT)
	*(sp++)=tos;
	tos=(cell_t)*(ip++);
	goto next;

m4_prim(docol,DOCOL)
	*(rp++)=(cell_t)ip;
	ip=(void ***)wp+1;
	goto next;

m4_prim(exit,EXIT)
	ip=(void ***)*(--rp);
	goto next;

m4_prim(add,+)
	tos+=*(--sp);
	goto next;
}
