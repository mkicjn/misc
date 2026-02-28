//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

/******** Lexer ********/

#define SPACE_CHARS \
	"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10" \
	"\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20"

#define LETTER_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define NUMBER_CHARS "0123456789"
#define WORD_CHARS   "_" LETTER_CHARS NUMBER_CHARS

#define FOREACH_TOKEN(X) \
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
#define DEF_ENUM_VAL(e, f, ...) TOK_##e,
	FOREACH_TOKEN(DEF_ENUM_VAL)
	NUM_TOKS
};
const char *tok_desc[NUM_TOKS] = {
	"ERROR",
	"EOF",
#define DEF_DESC_VAL(e, f, ...) #e,
	FOREACH_TOKEN(DEF_DESC_VAL)
};

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

enum tok_type lex(void)
{
	tok_buf[0] = (tok_len > 0 ? tok_buf[tok_len] : getchar());
	tok_len = 0;
	while (tok_len < sizeof(tok_buf) - 2) {
		int c = getchar();
		tok_buf[++tok_len] = (c == EOF ? '\0' : c);
		tok_buf[tok_len + 1] = '\0';

#define TRY_LEX(e, f, ...) \
		if (f(__VA_ARGS__)) \
			return TOK_##e;
		FOREACH_TOKEN(TRY_LEX)
	}
	return tok_buf[0] == '\0' ? TOK_EOF : TOK_ERROR;
}


/******** Parser ********/

#define panic(...) do {printf(__VA_ARGS__); exit(1);} while (0)

#define SYMS_SPACE 100000
char syms[SYMS_SPACE] = {0};
char *intern(const char *str, size_t len)
{
	char *sym;
	for (sym = syms; sym[0] > 0; sym += sym[0] + 2)
		if (sym[0] == len && memcmp(&sym[1], str, len) == 0)
			return &sym[1];
	sym[0] = len;
	memcpy(&sym[1], str, len);
	sym[1 + len] = '\0';
	return &sym[1];
}

enum tok_type cur_tok = TOK_ERROR; // TODO: Move this to lexer

bool have(enum tok_type expected)
{
	return cur_tok == expected;
}

void expect(enum tok_type expected)
{
	if (!have(expected))
		panic("Expected %s, got %s\n", tok_desc[expected], tok_desc[cur_tok]);
}

void consume(enum tok_type expected)
{
	expect(expected);
	do {
		cur_tok = lex();
	} while (cur_tok == TOK_SPACE);
	//printf("%s: %.*s\n", tok_desc[cur_tok], tok_len, tok_buf);
}


// Grammar
#define TERMS_SPACE 100000
struct term {
	enum {
		TERM_VAR,
		TERM_ABS,
		TERM_APP,
	} type;
	union {
		struct {
			char *name;
		} var;
		struct {
			struct term *var;
			struct term *body;
		} abs;
		struct {
			struct term *func;
			struct term *arg;
		} app;
	} as;
} terms[TERMS_SPACE];
struct term *next_term = terms;

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

struct term *parse_basic_term(void)
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
	struct term *t = parse_basic_term();
	if (!t)
		panic("Unexpected %s\n", tok_desc[cur_tok]);

	for (;;) {
		struct term *t2 = parse_basic_term();
		if (t2 == NULL)
			break;

		struct term *app = next_term++;
		app->type = TERM_APP;
		app->as.app.func = t;
		app->as.app.arg = t2;
		t = app;
	}
	return t;
}


/******** Main ********/

void lex_test(void)
{
	for (;;) {
		enum tok_type tok = lex();
		if (tok == TOK_SPACE)
			continue;
		printf("%s: %.*s\n", tok_desc[tok], tok_len, tok_buf);
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
		printf("(Var %s)", t->as.var.name);
		break;
	case TERM_ABS:
		printf("(Abs ");
		print_term(t->as.abs.var);
		printf(" ");
		print_term(t->as.abs.body);
		printf(")");
		break;
	case TERM_APP:
		printf("(App ");
		print_term(t->as.app.func);
		printf(" ");
		print_term(t->as.app.arg);
		printf(")");
		break;
	}
}

void parse_test(void)
{
	consume(TOK_ERROR);
	print_term(parse_term());
	printf("\n");
	consume(TOK_EOF);
}

int main()
{
	parse_test();
}
