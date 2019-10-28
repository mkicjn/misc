#include <stdio.h>
#include <termios.h>
typedef long cell_t;

struct link {
	struct link *prev;
	char *name;
	cell_t namelen;
};

#define FTH_REGS ( \
		void (**ip[])(), \
		cell_t *sp, \
		cell_t *rp, \
		union workreg w, \
		cell_t tos \
	)
union workreg {
	void (**p) FTH_REGS;
	cell_t c;
};

struct primitive {
	struct link link;
	void (**xt[1]) FTH_REGS;
};

static inline void next FTH_REGS
{
	w.p = *(ip++);
	(*w.p)(ip, sp, rp, w, tos);
}

void bye_code FTH_REGS
{
	return;
}
struct primitive bye_def = {
	.link = {
		.prev = (struct link *)NULL,
		.name = "BYE",
		.namelen = 3,
	},
	.xt = {(void *)bye_code},
};

void dolit_code FTH_REGS
{
	*(sp++) = tos;
	tos = (cell_t)*(ip++);
	next(ip, sp, rp, w, tos);
}
struct primitive dolit_def = {
	.link = {
		.prev = (struct link *)NULL,
		.name = "DOLIT",
		.namelen = 5,
	},
	.xt = {(void *)dolit_code},
};

void docol_code FTH_REGS
{
	*(rp++) = (cell_t)ip;
	ip = (void *)(w.p + 1);
	next(ip, sp, rp, w, tos);
}
struct primitive docol_def = {
	.link = {
		.prev = (struct link *)NULL,
		.name = "DOCOL",
		.namelen = 5,
	},
	.xt = {(void *)docol_code},
};

void exit_code FTH_REGS
{
	ip = (void *)*(--rp);
	next(ip, sp, rp, w, tos);
}
struct primitive exit_def = {
	.link = {
		.prev = (struct link *)NULL,
		.name = "EXIT",
		.namelen = 4,
	},
	.xt = {(void *)exit_code},
};

void add_code FTH_REGS
{
	tos += *(--sp);
	next(ip, sp, rp, w, tos);
}
struct primitive add_def = {
	.link = {
		.prev = (struct link *)NULL,
		.name = "+",
		.namelen = 1,
	},
	.xt = {(void *)add_code},
};

void interp(void (**ip[])())
{
	cell_t sp[64];
	cell_t rp[32];
	union workreg w = {.p = NULL};
	cell_t tos = 0;
	next(ip, sp, rp, w, tos);
}

int main(int argc, char **argv)
{
	/* Temporary */
	typedef void (**f)();
	static void (**inc_xt[])() = {
		(f)docol_code, // !!
		(f)dolit_def.xt, (f)1,
		(f)add_def.xt,
		(f)exit_def.xt,
	};
	static void (**p[])() = {
		(f)dolit_def.xt, (f)2,
		(f)inc_xt,
		(f)bye_def.xt,
	};
	/* ********* */
	interp(p);
	return 0;
}
