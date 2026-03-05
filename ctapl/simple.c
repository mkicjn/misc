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
	X(TRUE, exact_match, "true") \
	X(FALSE, exact_match, "false") \
	X(IF, exact_match, "if") \
	X(THEN, exact_match, "then") \
	X(ELSE, exact_match, "else") \
	X(ZERO, exact_match, "zero") \
	X(SUCC, exact_match, "succ") \
	X(PRED, exact_match, "pred") \
	X(ISZERO, exact_match, "iszero") \
	X(LPAREN, exact_match, "(") \
	X(RPAREN, exact_match, ")") \
	X(BOOL, exact_match, "Bool") \
	X(NAT, exact_match, "Nat") \
	X(ARROW, exact_match, "->") \
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
		TERM_COND,
		TERM_TRUE,
		TERM_FALSE,
		TERM_BOOL,
		TERM_ZERO,
		TERM_SUCC,
		TERM_PRED,
		TERM_ISZERO,
		TERM_NAT,
		TERM_ARROW,
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
			struct term *type;
			struct term *body;
		} abs, nabs;
		struct {
			struct term *fun;
			struct term *arg;
		} app;
		struct {
			struct term *test;
			struct term *when_t;
			struct term *when_f;
		} cond;
		struct {
			struct term *from;
			struct term *to;
		} arrow;
		struct {
			struct term *arg;
		} succ, pred, iszero;
	} as;
} terms[2][TERMS_SPACE];
size_t bank = 0;
struct term *next_term = terms[0];

static inline struct term *new_term(enum term_type ty)
{
	struct term *t = next_term++;
	t->type = ty;
	return t;
}

struct term *parse_term(void);

struct term *parse_type(void)
{
	struct term *ty;
	switch (tok) {
	case TOK_LPAREN:
		consume(TOK_LPAREN);
		ty = parse_type();
		consume(TOK_RPAREN);
		break;
	case TOK_BOOL:
		consume(TOK_BOOL);
		ty = new_term(TERM_BOOL);
		break;
	case TOK_NAT:
		consume(TOK_NAT);
		ty = new_term(TERM_NAT);
		break;
	default:
		PANIC("Unexpected token type %s\n", tok_desc[tok]);
		return NULL;
	}

	// Handle arrow types (right-associative)
	if (have(TOK_ARROW)) {
		consume(TOK_ARROW);
		struct term *from = ty;
		struct term *to = parse_type();
		ty = new_term(TERM_ARROW);
		ty->as.arrow.from = from;
		ty->as.arrow.to = to;
	}

	return ty;
}

struct term *parse_var(void)
{
	expect(TOK_WORD);
	char *x = intern(tok_buf, tok_len);
	consume(TOK_WORD);

	struct term *t = new_term(TERM_VAR);
	t->as.var.name = x;
	return t;
}

struct term *parse_abs(void)
{
	consume(TOK_LAMBDA);
	struct term *x = parse_var();
	consume(TOK_COLON);
	struct term *ty = parse_type();
	consume(TOK_DOT);
	struct term *t1 = parse_term();

	struct term *t = new_term(TERM_ABS);
	t->as.abs.var = x;
	t->as.abs.type = ty;
	t->as.abs.body = t1;
	return t;
}

struct term *parse_cond(void)
{
	consume(TOK_IF);
	struct term *test = parse_term();
	consume(TOK_THEN);
	struct term *when_t = parse_term();
	consume(TOK_ELSE);
	struct term *when_f = parse_term();

	struct term *t = new_term(TERM_COND);
	t->as.cond.test = test;
	t->as.cond.when_t = when_t;
	t->as.cond.when_f = when_f;
	return t;
}

struct term *parse_base_term(void)
{
	struct term *t;
	switch (tok) {
	case TOK_LAMBDA:
		return parse_abs();
	case TOK_WORD:
		return parse_var();
	case TOK_LPAREN:
		consume(TOK_LPAREN);
		t = parse_term();
		consume(TOK_RPAREN);
		return t;
	case TOK_IF:
		return parse_cond();
	case TOK_TRUE:
		consume(TOK_TRUE);
		return new_term(TERM_TRUE);
	case TOK_FALSE:
		consume(TOK_FALSE);
		return new_term(TERM_FALSE);
	case TOK_ZERO:
		consume(TOK_ZERO);
		return new_term(TERM_ZERO);
	case TOK_SUCC:
		consume(TOK_SUCC);
		t = new_term(TERM_SUCC);
		t->as.succ.arg = parse_term();
		return t;
	case TOK_PRED:
		consume(TOK_PRED);
		t = new_term(TERM_PRED);
		t->as.pred.arg = parse_term();
		return t;
	case TOK_ISZERO:
		consume(TOK_ISZERO);
		t = new_term(TERM_ISZERO);
		t->as.iszero.arg = parse_term();
		return t;
	default:
		return NULL;
	}
}

struct term *parse_term(void)
{
	struct term *t1 = parse_base_term();
	if (t1 == NULL)
		PANIC("Unexpected token type %s\n", tok_desc[tok]);

	// Handle application (left-associative)
	for (;;) {
		struct term *t2 = parse_base_term();
		if (t2 == NULL)
			break;

		struct term *app = new_term(TERM_APP);
		app->as.app.fun = t1;
		app->as.app.arg = t2;
		t1 = app;
	}
	return t1;
}


/******** Name removal ********/

#define UNEXPECTED_TERM(t) PANIC("%s: Unexpected term type %d\n", __func__, (t)->type)
// ^ Simply print the enum's integer value, since this is probably programmer error

void remove_name(struct term *t, struct term *x, intptr_t level)
{
	switch (t->type) {
	case TERM_VAR:
		if (t->as.var.name != x->as.var.name)
			return;
		t->type = TERM_NVAR;
		t->as.nvar.idx = level;
		return;
	case TERM_ABS:
		if (t->as.abs.var->as.var.name == x->as.var.name)
			return;
		remove_name(t->as.abs.body, x, level + 1);
		return;
	case TERM_APP:
		remove_name(t->as.app.fun, x, level);
		remove_name(t->as.app.arg, x, level);
		return;
	case TERM_NVAR:
		return;
	case TERM_NABS:
		return;
	case TERM_COND:
		remove_name(t->as.cond.test, x, level);
		remove_name(t->as.cond.when_t, x, level);
		remove_name(t->as.cond.when_f, x, level);
		return;
	case TERM_TRUE:
		return;
	case TERM_FALSE:
		return;
	case TERM_ZERO:
		return;
	case TERM_SUCC:
		remove_name(t->as.succ.arg, x, level);
		return;
	case TERM_PRED:
		remove_name(t->as.pred.arg, x, level);
		return;
	case TERM_ISZERO:
		remove_name(t->as.iszero.arg, x, level);
		return;
	default:
		UNEXPECTED_TERM(t);
		return;
	}
}

void remove_names(struct term *t)
{
	switch (t->type) {
	case TERM_VAR:
		return;
	case TERM_ABS:
		remove_name(t->as.abs.body, t->as.abs.var, 0);
		remove_names(t->as.abs.body);
		t->as.abs.var = NULL;
		t->type = TERM_NABS;
		return;
	case TERM_APP:
		remove_names(t->as.app.fun);
		remove_names(t->as.app.arg);
		return;
	case TERM_NVAR:
		return;
	case TERM_NABS:
		return;
	case TERM_COND:
		remove_names(t->as.cond.test);
		remove_names(t->as.cond.when_t);
		remove_names(t->as.cond.when_f);
		return;
	case TERM_TRUE:
		return;
	case TERM_FALSE:
		return;
	case TERM_ZERO:
		return;
	case TERM_SUCC:
		remove_names(t->as.succ.arg);
		return;
	case TERM_PRED:
		remove_names(t->as.pred.arg);
		return;
	case TERM_ISZERO:
		remove_names(t->as.iszero.arg);
		return;
	default:
		UNEXPECTED_TERM(t);
		return;
	}
}


/******** Term Reduction ********/

struct term *shift(struct term *t, int d, int c)
{
	// Makes a deep copy of t, shifting free de Bruijn indices by d (initially, c = 0)
	struct term *new = new_term(t->type);
	switch (t->type) {
	// Nameless variables and abstractions are special cases
	case TERM_NVAR:
		new->as.nvar.idx = t->as.nvar.idx;
		if (t->as.nvar.idx >= c)
			new->as.nvar.idx += d;
		return new;
	case TERM_NABS:
		new->as.nabs.type = shift(t->as.nabs.type, 0, c + 1);
		new->as.nabs.body = shift(t->as.nabs.body, d, c + 1);
		return new;
	// Most cases will simply involve recursing over subterms or otherwise copying data
	case TERM_VAR:
		new->as.var.name = t->as.var.name;
		return new;
	case TERM_APP:
		new->as.app.fun = shift(t->as.app.fun, d, c);
		new->as.app.arg = shift(t->as.app.arg, d, c);
		return new;
	case TERM_COND:
		new->as.cond.test = shift(t->as.cond.test, d, c);
		new->as.cond.when_t = shift(t->as.cond.when_t, d, c);
		new->as.cond.when_f = shift(t->as.cond.when_f, d, c);
		return new;
	case TERM_TRUE:
		return new;
	case TERM_FALSE:
		return new;
	case TERM_ZERO:
		return new;
	case TERM_SUCC:
		new->as.succ.arg = shift(t->as.succ.arg, d, c);
		return new;
	case TERM_PRED:
		new->as.pred.arg = shift(t->as.succ.arg, d, c);
		return new;
	case TERM_ISZERO:
		new->as.iszero.arg = shift(t->as.succ.arg, d, c);
		return new;
	// This function must also handle types, since it is used for copying with d = 0
	case TERM_BOOL:
		return new;
	case TERM_NAT:
		return new;
	case TERM_ARROW:
		new->as.arrow.from = shift(t->as.arrow.from, 0, c);
		new->as.arrow.to = shift(t->as.arrow.to, 0, c);
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

	struct term *new = new_term(t->type);
	switch (t->type) {
	// Nameless variables and abstractions are special cases
	case TERM_NVAR:
		new->as.nvar.idx = t->as.nvar.idx;
		return new;
	case TERM_NABS:
		new->as.nabs.type = shift(t->as.nabs.type, 0, 0);
		new->as.nabs.body = subst(t->as.nabs.body, k + 1, shift(v, 1, 0));
		return new;
	// Most cases will simply involve recursing over subterms or otherwise copying data
	case TERM_VAR:
		new->as.var.name = t->as.var.name;
		return new;
	case TERM_APP:
		new->as.app.fun = subst(t->as.app.fun, k, v);
		new->as.app.arg = subst(t->as.app.arg, k, v);
		return new;
	case TERM_COND:
		new->as.cond.test = subst(t->as.cond.test, k, v);
		new->as.cond.when_t = subst(t->as.cond.when_t, k, v);
		new->as.cond.when_f = subst(t->as.cond.when_f, k, v);
		return new;
	case TERM_TRUE:
		return new;
	case TERM_FALSE:
		return new;
	case TERM_ZERO:
		return new;
	case TERM_SUCC:
		new->as.succ.arg = subst(t->as.succ.arg, k, v);
		return new;
	case TERM_PRED:
		new->as.pred.arg = subst(t->as.pred.arg, k, v);
		return new;
	case TERM_ISZERO:
		new->as.iszero.arg = subst(t->as.iszero.arg, k, v);
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
	case TERM_COND:
		if (reduce(&t->as.cond.test))
			return true;
		if (t->as.cond.test->type == TERM_TRUE) {
			*tp = t->as.cond.when_t;
			return true;
		} else if (t->as.cond.test->type == TERM_FALSE) {
			*tp = t->as.cond.when_f;
			return true;
		} else {
			if (reduce(&t->as.cond.when_t))
				return true;
			if (reduce(&t->as.cond.when_f))
				return true;
		}
		return false;
	case TERM_SUCC:
		if (reduce(&t->as.succ.arg))
			return true;
		return false;
	case TERM_PRED:
		// TODO: Check properly for numeric values
		if (reduce(&t->as.pred.arg))
			return true;
		if (t->as.pred.arg->type == TERM_ZERO) {
			*tp = t->as.pred.arg;
			return true;
		} else if (t->as.pred.arg->type == TERM_SUCC) {
			*tp = t->as.pred.arg->as.succ.arg;
			return true;
		}
		return false;
	case TERM_ISZERO:
		if (reduce(&t->as.iszero.arg))
			return true;
		if (t->as.iszero.arg->type == TERM_ZERO) {
			*tp = new_term(TERM_TRUE);
			return true;
		} else if (t->as.iszero.arg->type == TERM_SUCC) {
			*tp = new_term(TERM_FALSE);
			return true;
		}
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


/******** Typing ********/

bool type_eq(struct term *ty1, struct term *ty2)
{
	if (ty1 == NULL || ty2 == NULL)
		return false;

	if (ty1->type != ty2->type)
		return false;

	switch (ty1->type) {
	case TERM_BOOL:
		return true;
	case TERM_NAT:
		return true;
	case TERM_ARROW:
		if (!type_eq(ty1->as.arrow.from, ty2->as.arrow.from))
			return false;
		if (!type_eq(ty1->as.arrow.to, ty2->as.arrow.to))
			return false;
		return true;
	default:
		UNEXPECTED_TERM(ty1);
		return false;
	}
}

struct type_ctx {
	struct term *type;
	struct type_ctx *next;
};

struct term *type_ctx_lookup(struct type_ctx *gamma, intptr_t k)
{
	if (k < 0 || gamma == NULL)
		return NULL;
	if (k == 0)
		return gamma->type;
	return type_ctx_lookup(gamma->next, k - 1);
}

struct term *type_of(struct term *t, struct type_ctx *gamma)
{
	// The typing context Γ is a linked list generated on the stack (initially, gamma = NULL)
	struct type_ctx gamma_prime;
	gamma_prime.next = gamma;

	struct term *ty, *ty1, *ty2, *ty3;
	switch (t->type) {
	case TERM_VAR:
		return NULL;
	case TERM_NVAR:
		return type_ctx_lookup(gamma, t->as.nvar.idx);
	case TERM_NABS:
		gamma_prime.type = t->as.nabs.type;
		ty1 = t->as.nabs.type;
		ty2 = type_of(t->as.nabs.body, &gamma_prime);
		if (ty2 == NULL)
			return NULL;
		ty = new_term(TERM_ARROW);
		ty->as.arrow.from = ty1;
		ty->as.arrow.to = ty2;
		return ty;
	case TERM_APP:
		ty1 = type_of(t->as.app.fun, gamma);
		ty2 = type_of(t->as.app.arg, gamma);
		if (ty1 == NULL || ty1->type != TERM_ARROW)
			return NULL;
		if (type_eq(ty1->as.arrow.from, ty2))
			return ty1->as.arrow.to;
		return NULL;
	case TERM_TRUE:
		return new_term(TERM_BOOL);
	case TERM_FALSE:
		return new_term(TERM_BOOL);
	case TERM_COND:
		ty1 = type_of(t->as.cond.test, gamma);
		if (ty1 == NULL || ty1->type != TERM_BOOL)
			return NULL;
		ty2 = type_of(t->as.cond.when_t, gamma);
		ty3 = type_of(t->as.cond.when_f, gamma);
		if (!type_eq(ty2, ty3))
			return NULL;
		return ty2;
	case TERM_ZERO:
		return new_term(TERM_NAT);
	case TERM_SUCC:
		ty1 = type_of(t->as.succ.arg, gamma);
		if (ty1 == NULL || ty1->type != TERM_NAT)
			return NULL;
		return ty1;
	case TERM_PRED:
		ty1 = type_of(t->as.pred.arg, gamma);
		if (ty1 == NULL || ty1->type != TERM_NAT)
			return NULL;
		return ty1;
	case TERM_ISZERO:
		ty1 = type_of(t->as.iszero.arg, gamma);
		if (ty1 == NULL || ty1->type != TERM_NAT)
			return NULL;
		return new_term(TERM_BOOL);
	default:
		UNEXPECTED_TERM(t);
		return NULL;
	}
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
		printf(" : ");
		print_term(t->as.abs.type);
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
		print_term(t->as.nabs.type);
		printf(" ");
		print_term(t->as.nabs.body);
		printf(")");
		break;
	case TERM_COND:
		printf("(if ");
		print_term(t->as.cond.test);
		printf(" then ");
		print_term(t->as.cond.when_t);
		printf(" else ");
		print_term(t->as.cond.when_f);
		printf(")");
		break;
	case TERM_TRUE:
		printf("true");
		break;
	case TERM_FALSE:
		printf("false");
		break;
	case TERM_ZERO:
		printf("zero");
		break;
	case TERM_SUCC:
		printf("(succ ");
		print_term(t->as.succ.arg);
		printf(")");
		break;
	case TERM_PRED:
		printf("(pred ");
		print_term(t->as.pred.arg);
		printf(")");
		break;
	case TERM_ISZERO:
		printf("(iszero ");
		print_term(t->as.iszero.arg);
		printf(")");
		break;
	case TERM_BOOL:
		printf("Bool");
		break;
	case TERM_NAT:
		printf("Nat");
		break;
	case TERM_ARROW:
		printf("(");
		print_term(t->as.arrow.from);
		printf(" -> ");
		print_term(t->as.arrow.to);
		printf(")");
		break;
	default:
		UNEXPECTED_TERM(t);
		break;
	}
}

int main()
{
	lex();
	struct term *t = parse_term();
	consume(TOK_EOF);

	printf("⊢ ");
	print_term(t);
	printf(" : ");

	remove_names(t);
	struct term *ty = type_of(t, NULL);
	print_term(ty);
	printf("\n");

	do {
		t = gc(t);
		print_term(t);
		printf("\n");
	} while (reduce(&t));

	return 0;
}

// Typing examples to try:
//
// Bool -> Bool
// λx:Bool. if x then true else false
//
// Bool -> Bool -> Bool
// λx:Bool. if x then (λy:Bool. if y then false else true) else (λz:Bool. z)
//
// (Bool -> Bool) -> Bool -> Bool
// λf : Bool->Bool. λx: Bool. if f x then false else true
//
// Bool
// (λf : Bool->Bool. λx: Bool. if f x then false else true) (λx: Bool. x) false
