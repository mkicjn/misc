//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/******** Lexer ********/

#define SPACE_CHARS \
	"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10" \
	"\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20"

#define LETTER_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define NUMBER_CHARS "0123456789"
#define WORD_CHARS   "_" LETTER_CHARS NUMBER_CHARS

#define FOREACH_TOK_TYPE(X) \
	X(LAMBDA, exact_match, "λ") \
	X(COLON, exact_match, ":") \
	X(DOT, exact_match, ".") \
	X(SUCC, exact_match, "succ") \
	X(ZERO, exact_match, "0") \
	X(LPAREN, exact_match, "(") \
	X(RPAREN, exact_match, ")") \
	X(NUMBER, sequence_of, NUMBER_CHARS) \
	X(WORD, sequence_of, WORD_CHARS) \
	X(SPACE, sequence_of, SPACE_CHARS)

enum tok_type {
	TOK_ERROR,
	TOK_EOF,
#define DEF_TOK_ENUM(e, f, ...) TOK_##e,
	FOREACH_TOK_TYPE(DEF_TOK_ENUM)
	NUM_TOK_TYPES
} tok = TOK_ERROR;

#define MAX_TOK_LEN 127
char tok_buf[MAX_TOK_LEN] = {0};
size_t tok_len = 0;

bool exact_match(const char *str)
{
	return tok_len >= strlen(str) && strncmp(str, tok_buf, tok_len) == 0;
}

bool sequence_of(const char *cs)
{
	return tok_len >= 1 && strspn(tok_buf, cs) == tok_len;
}

void lex(void)
{
	tok_buf[0] = (tok_len > 0 ? tok_buf[tok_len] : getchar());
	tok_len = 0;
	while (tok_len < sizeof(tok_buf) - 2) {
		int c = getchar();
		tok_buf[++tok_len] = (c == EOF ? '\0' : c);
		tok_buf[tok_len + 1] = '\0';

#define TRY_LEX(e, f, ...) \
		if (f(__VA_ARGS__)) { \
			tok = TOK_##e;\
			return; \
		}
		FOREACH_TOK_TYPE(TRY_LEX)
	}
	tok = (tok_buf[0] == '\0' ? TOK_EOF : TOK_ERROR);
}

/******** String Interning ********/

#define SYMS_SPACE 100000
char syms[SYMS_SPACE] = {0};
char *intern(const char *str, size_t len)
{
	char l = len > 127 ? 127 : len;
	char *sym;
	for (sym = syms; sym[0] > 0; sym += sym[0] + 2)
		if (sym[0] == l && memcmp(&sym[1], str, l) == 0)
			return &sym[1];
	sym[0] = l;
	memcpy(&sym[1], str, l);
	sym[1 + l] = '\0';
	return &sym[1];
}


/******** Parser ********/

// (Basic utilities)
#define PANIC(...) do {printf(__VA_ARGS__); exit(1);} while (0)

const char *tok_desc[NUM_TOK_TYPES] = {
	"ERROR",
	"EOF",
#define DEF_TOK_DESC(e, f, ...) #e,
	FOREACH_TOK_TYPE(DEF_TOK_DESC)
};

static inline bool have(enum tok_type expected)
{
	return tok == expected;
}

static inline void expect(enum tok_type expected)
{
	if (!have(expected))
		PANIC("Expected %s, got %s\n", tok_desc[expected], tok_desc[tok]);
}

void consume(enum tok_type expected)
{
	expect(expected);
	do {
		lex();
	} while (tok == TOK_SPACE);
	//printf("%s: %.*s\n", tok_desc[tok], tok_len, tok_buf);
}


// (Grammar)
#define TERMS_SPACE 100000
struct term {
	enum term_type {
		TERM_VAR,
		TERM_ABS,
		TERM_APP,
		TERM_NVAR,
		TERM_NABS,
		NUM_TERM_TYPES
	} type;
	union {
		struct {
			char *name;
		} var;
		struct {
			intptr_t idx;
		} nvar;
		struct {
			struct term *var;
			struct term *body;
		} abs, nabs;
		struct {
			struct term *fun;
			struct term *arg;
		} app;
	} as;
} terms[2][TERMS_SPACE];
size_t bank = 0;
struct term *next_term = terms[0];

struct term *parse_term(void);

struct term *parse_var(void)
{
	expect(TOK_WORD);
	char *x = intern(tok_buf, tok_len);
	consume(TOK_WORD);

	struct term *t = next_term++;
	t->type = TERM_VAR;
	t->as.var.name = x;
	return t;
}

struct term *parse_abs(void)
{
	consume(TOK_LAMBDA);
	struct term *x = parse_var();
	consume(TOK_DOT);
	struct term *t1 = parse_term();

	struct term *t = next_term++;
	t->type = TERM_ABS;
	t->as.abs.var = x;
	t->as.abs.body = t1;
	return t;
}

struct term *parse_base_term(void)
{
	if (have(TOK_LAMBDA)) {
		return parse_abs();
	} else if (have(TOK_WORD)) {
		return parse_var();
	} else if (have(TOK_LPAREN)) {
		consume(TOK_LPAREN);
		struct term *t = parse_term();
		consume(TOK_RPAREN);
		return t;
	}
	return NULL;
}

struct term *parse_term(void)
{
	struct term *t = parse_base_term();
	if (!t)
		PANIC("Unexpected token type %s\n", tok_desc[tok]);

	for (;;) {
		struct term *t2 = parse_base_term();
		if (t2 == NULL)
			break;

		struct term *app = next_term++;
		app->type = TERM_APP;
		app->as.app.fun = t;
		app->as.app.arg = t2;
		t = app;
	}
	return t;
}


/******** Name removal ********/

#define UNEXPECTED_TERM(t) PANIC("%s: Unexpected term type %d\n", __func__, (t)->type)
// ^ Simply print the enum's integer value, since this is probably programmer error

void remove_name(struct term *t, struct term *x, intptr_t level)
{
	switch (t->type) {
	case TERM_VAR:
		if (t->as.var.name != x->as.var.name)
			break;
		t->type = TERM_NVAR;
		t->as.nvar.idx = level;
		break;
	case TERM_ABS:
		if (t->as.abs.var->as.var.name == x->as.var.name)
			break;
		remove_name(t->as.abs.body, x, level + 1);
		break;
	case TERM_APP:
		remove_name(t->as.app.fun, x, level);
		remove_name(t->as.app.arg, x, level);
		break;
	case TERM_NVAR:
		break;
	case TERM_NABS:
		break;
	default:
		UNEXPECTED_TERM(t);
		break;
	}
}

void remove_names(struct term *t)
{
	switch (t->type) {
	case TERM_VAR:
		break;
	case TERM_ABS:
		remove_name(t->as.abs.body, t->as.abs.var, 0);
		remove_names(t->as.abs.body);
		t->as.abs.var = NULL;
		t->type = TERM_NABS;
		break;
	case TERM_APP:
		remove_names(t->as.app.fun);
		remove_names(t->as.app.arg);
		break;
	case TERM_NVAR:
		break;
	case TERM_NABS:
		break;
	default:
		UNEXPECTED_TERM(t);
		break;
	}
}


/******** Term Reduction ********/

struct term *shift(struct term *t, int d, int c)
{
	// Makes a deep copy of t, shifted by d (initially, c = 0)
	struct term *new = next_term++;
	new->type = t->type;
	switch (t->type) {
	case TERM_VAR:
		new->as.var.name = t->as.var.name;
		return new;
	case TERM_NVAR:
		new->as.nvar.idx = t->as.nvar.idx;
		if (t->as.nvar.idx >= c)
			new->as.nvar.idx += d;
		return new;
	case TERM_NABS:
		new->as.nabs.body = shift(t->as.nabs.body, d, c + 1);
		return new;
	case TERM_APP:
		new->as.app.fun = shift(t->as.app.fun, d, c);
		new->as.app.arg = shift(t->as.app.arg, d, c);
		return new;
	default:
		UNEXPECTED_TERM(t);
		return NULL;
	}
}

struct term *subst(struct term *t, int k, struct term *v)
{
	// Makes a deep copy of t, but de Bruijn index k is substituted with v
	if (t->type == TERM_NVAR && t->as.nvar.idx == k)
		return v;

	struct term *new = next_term++;
	new->type = t->type;
	switch (t->type) {
	case TERM_VAR:
		new->as.var.name = t->as.var.name;
		return new;
	case TERM_NVAR:
		new->as.nvar.idx = t->as.nvar.idx;
		return new;
	case TERM_NABS:
		new->as.nabs.body = subst(t->as.nabs.body, k + 1, shift(v, 1, 0));
		return new;
	case TERM_APP:
		new->as.app.fun = subst(t->as.app.fun, k, v);
		new->as.app.arg = subst(t->as.app.arg, k, v);
		return new;
	default:
		UNEXPECTED_TERM(t);
		return NULL;
	}
}

bool reduce(struct term **tp)
{
	// Perform a single normal-order reduction
	struct term *t = *tp;
	switch (t->type) {
	case TERM_NABS:
		if (reduce(&t->as.nabs.body))
			return true;
		return false;
	case TERM_APP:
		if (t->as.app.fun->type == TERM_NABS) {
			struct term *f = t->as.app.fun;
			struct term *a = t->as.app.arg;
			a = shift(a, 1, 0);
			t = subst(f->as.nabs.body, 0, a);
			*tp = shift(t, -1, 0);
			return true;
		}
		if (reduce(&t->as.app.fun))
			return true;
		if (reduce(&t->as.app.arg))
			return true;
		return false;
	default:
		return false;
	}
}

struct term *gc(struct term *t)
{
	// Simplistic copying GC
	bank = 1 - bank;
	next_term = terms[bank];
	return shift(t, 0, 0);
}


/******** Main ********/

void lex_test(void)
{
	for (;;) {
		lex();
		if (tok == TOK_SPACE)
			continue;
		printf("%s: %.*s\n", tok_desc[tok], (int)tok_len, tok_buf);
		if (tok == TOK_WORD)
			printf("%p\n", intern(tok_buf, tok_len));
		if (tok == TOK_ERROR || tok == TOK_EOF)
			break;
	}
}

void print_term(struct term *t)
{
	if (!t) {
		printf("NULL");
		return;
	}

	switch (t->type) {
	case TERM_VAR:
		printf("%s", t->as.var.name);
		break;
	case TERM_ABS:
		printf("(λ");
		print_term(t->as.abs.var);
		printf(". ");
		print_term(t->as.abs.body);
		printf(")");
		break;
	case TERM_APP:
		printf("(");
		print_term(t->as.app.fun);
		printf(" ");
		print_term(t->as.app.arg);
		printf(")");
		break;
	case TERM_NVAR:
		printf("%ld", t->as.nvar.idx);
		break;
	case TERM_NABS:
		printf("(λ ");
		print_term(t->as.nabs.body);
		printf(")");
		break;
	default:
		UNEXPECTED_TERM(t);
		break;
	}
}

struct term *parse_test(void)
{
	consume(TOK_ERROR);
	struct term *t = parse_term();
	consume(TOK_EOF);
	return t;
}

int main()
{
	struct term *t = parse_test();

	print_term(t);
	printf("\n");

	remove_names(t);
	do {
		t = gc(t);
		print_term(t);
		printf("\n");
	} while (reduce(&t));

	return 0;
}

// Arithmetic examples to try:
//
// 2+2=4
// (λplus. λc2. plus c2 c2) (λm.λn.λs.λz. m s (n s z)) (λs.λz. s (s z))
//
// 2*3=6
// (λtimes. λc2. λc3. times c2 c3) ((λplus. λc0. λm. λn. m (plus n) c0) (λm.λn.λs.λz. m s (n s z)) (λs.λz.z)) (λs.λz. s (s z)) (λs.λz. s (s (s z)))
//
// 2^3=8
// (λpow. λc2. λc3. pow c2 c3) (λm.λn. n m) (λs.λz. s (s z)) (λs.λz. s (s (s z)))
