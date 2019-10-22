#include <termios.h>
#include <stdio.h>
typedef long cell_t;

typedef struct link_s {
	struct link_s *prev;
	char *name;
	cell_t namelen;
} link_t;

typedef void (*vf)();
union wr {vf *p; cell_t c;};
#define FTH_REGS (vf **ip, cell_t *sp, cell_t *rp, union wr w, cell_t tos)

typedef struct {
	struct link_s link;
	vf **xt[1];
} prim_t;

static inline void next FTH_REGS
{
	w.p=*(ip++);
	(*w.p)(ip,sp,rp,w,tos);
}

void bye_code FTH_REGS
{
	return;
}
prim_t bye_def={.link={.prev=NULL,.name="BYE",.namelen=3},.xt={(vf **)bye_code}};

int main(int argc,char **argv)
{
	/*
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
	*/
}
