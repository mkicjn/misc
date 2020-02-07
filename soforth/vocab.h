#ifndef VOCAB_H
#define VOCAB_H

struct fthdef {
	struct fthdef *prev;
	char *name;
	bool imm : 1;
	cell len : 8*sizeof(cell)-1;
	void *cf;
	void **data[];
};

// Used in the VOCAB X macro below to generate fthdef structs
#define DEF(cn,pr,nm,im,cf,...) \
struct fthdef cn##_d = { \
	.prev = pr, \
	.name = nm, \
	.imm = im, \
	.len = COUNT(nm), \
	/*would do .cf = cf, but cf is out of scope*/ \
	.data = {__VA_ARGS__}, \
};

// Used in the VOCAB X macro below to generate a list of code fields
#define CF(cn,pr,nm,im,cf,...) cf,

// Used in the varargs of a VOCAB entry to compile code by hand
#define XT(cn) &cn##_d.cf
#define LIT(n) XT(lit),(void **)(n)

// VOCAB is an X-macro containing a list of Forth definitions
#define VOCAB(X) \
	X(bye,NULL,"BYE",0,&&bye_c) \
	X(lit,&bye_d,"LIT",0,&&lit_c) \
	X(exit,&lit_d,"EXIT",0,&&exit_c) \
	X(docol,&exit_d,"DOCOL",0,&&docol_c) \
	X(swap,&docol_d,"SWAP",0,&&swap_c) \
	X(add,&swap_d,"+",0,&&add_c) \
	X(cell,&add_d,"CELL",0,&&docol_c, \
		LIT(sizeof(cell)),XT(exit) \
	) \
// LAST_VOC is needed for the engine to know where to start filling in code fields
#define LAST_VOC &cell_d

#endif /*defined VOCAB_H*/
