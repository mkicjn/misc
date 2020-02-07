#define CF(cn,pr,nm,im,cf,...) cf,
#define DEF(cn,pr,nm,im,cf,...) \
struct fthdef cn##_d = { \
	.prev = pr, \
	.name = nm, \
	.imm = im, \
	.len = COUNT(nm), \
	/*.cf = cf,*/ \
	.data = {__VA_ARGS__}, \
};

#define XT(cn) &cn##_d.cf
#define LIT(n) XT(lit),(void **)(n)

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

#define LAST_VOC &cell_d
