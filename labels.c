typedef long cell_t;

typedef struct link_s {
	struct link_s *prev;
	char *name;
	cell_t len;
} link_t;

int main(int argc,char **argv)
{
	(void) argc; (void) argv;
#define next() wp=*(ip++); goto **wp;

	/* The below block would be automatically generated */
typedef struct {link_t link; void *xt[1];} bye_t;
static bye_t bye_def={{0,"BYE",3},{&&bye_code}}; (void) bye_def;
typedef struct {link_t link; void *xt[1];} dolit_t;
static dolit_t dolit_def={{0,"DOLIT",5},{&&dolit_code}}; (void) dolit_def;
typedef struct {link_t link; void *xt[1];} enter_t;
static enter_t enter_def={{0,"ENTER",5},{&&enter_code}}; (void) enter_def;
typedef struct {link_t link; void *xt[1];} exit_t;
static exit_t exit_def={{0,"EXIT",4},{&&exit_code}}; (void) exit_def;
typedef struct {link_t link; void *xt[1];} add_t;
static add_t add_def={{0,"+",1},{&&add_code}}; (void) add_def;

	static void **inc[]={&&enter_code,dolit_def.xt,(void *)1,add_def.xt,exit_def.xt};
	static void **test[]={dolit_def.xt,(void *)2,(void *)&inc,bye_def.xt};
	static cell_t stack[100];
	static cell_t rstack[100];

	register void ***ip=test;
	register void **wp=0;
	register cell_t *sp=stack;
	register cell_t *rp=rstack;
	register cell_t tos=0;
	next();

bye_code:
	return tos;

dolit_code:
	*(sp++)=tos;
	tos=(cell_t)*(ip++);
	next();

enter_code:
	*(rp++)=(cell_t)ip;
	ip=(void ***)wp+1;
	next();

exit_code:
	ip=(void ***)*(--rp);
	next();

add_code:
	tos+=*(--sp);
	next();
}
