#define EVAL1(...) __VA_ARGS__
#define BL
#define POSTPONE(x) x BL

#define TESTS(INI,X,FIN) \
	INI() \
	X(a,1) \
	X(b,2) \
	X(c,3) \
	FIN()

#define M_INI() POSTPONE(D) (NULL,
#define M(n,v) n,v) POSTPONE(D) (&n,
#define M_FIN() tail,NULL)

#define D(p,n,v) n = {.prev = p, .val = v};

TESTS(M_INI,M,M_FIN)

EVAL1(TESTS(M_INI,M,M_FIN))
