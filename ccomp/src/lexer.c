#include "lexer.h"

/*
 *		Helper Functions
 */

static int digit_val(int c)
{
	if ('0' <= c && c <= '9')
		return c - '0';
	else if ('A' <= c && c <= 'Z')
		return c - 'A' + 10;
	else if ('a' <= c && c <= 'z')
		return c - 'a' + 10;
	return -1;
}

static int digits(const char *str, int radix)
{
	int len = 0;
	while (str[len] != '\0') {
		int digit = digit_val(str[len]);
		if (!(0 <= digit && digit < radix))
			break;
		len++;
	}
	return len;
}

static inline bool casecmp(int a, int b)
{
	return a == b || a == (b^32);
}

static inline void skip_space(struct lexer *l)
{
	while (*l->pos && *l->pos <= ' ')
		l->pos++;
}

static bool refill(struct lexer *l)
{
	int rem = strlen(l->pos);
	int fetch_amt = l->cap - rem;
	memmove(l->buf, l->pos, rem);
	l->pos = l->buf;
	if (!fgets(l->buf + rem, fetch_amt, l->src))
		return false;
	return true;
}

/*
 *		Lexer Functions
 *
 * These have one job: Recognize a category of tokens.
 * On success, they modify the state of a struct lexer and return true.
 *
 * The next token is identified by three values in the lexer:
 *  1. (char *)l->pos -- a pointer to the first character of the token
 *  2. (int)l->len -- the length of the token
 *  3. (enum tok_type)l->type -- what type of token it is (see token.h)
 * When called, l->pos will point to the next unlexed location.
 * As such, l->pos should almost never be modified.
 */

static bool lex_key_op(struct lexer *l)
{
	enum tok_type t;
	for (t = 0; t < TOK_ERROR; t++) {
		// ^ TOK_ERROR marks end of keywords and operators,
		// since these categories are the simplest case lexically.
		if (memcmp(l->pos, tok_str[t], tok_len[t]) == 0)
			break;
	}
	if (t == TOK_ERROR)
		return false;
	l->type = t;
	l->len = tok_len[t];
	return true;
}

static bool lex_ident(struct lexer *l)
{
	int i = 0;
	char *s = l->pos;
	for (char c = s[i]; c > ' '; c = s[++i])
		if (!(isalpha(c) || c == '_' || (isdigit(c) && i>0)))
			break;
	if (i > 0) {
		l->type = TOK_IDENT;
		l->len = i;
		return true;
	}
	return false;
}

static bool lex_char(struct lexer *l)
{
	int len = 0;
	if (l->pos[0] != '\'' || l->pos[1] == '\'')
		return false;
	if (l->pos[1] == '\\') {
		if (l->pos[3] == '\'')
			len = 4;
	} else {
		if (l->pos[2] == '\'')
			len = 3;
	}
	if (len > 0) {
		l->type = TOK_LIT_CHAR;
		l->len = len;
		return true;
	}
	return false;
}

static bool lex_string(struct lexer *l)
{
	char *start, *end;
	refill(l); // Refill may fetch rest of string
	start = l->pos;
	end = start + 1;
	if (start[0] != '"')
		return false;
	do {
		end = strchr(end, '"');
	} while (end && end[-1] == '\\');
	if (end != NULL) {
		l->type = TOK_LIT_STR;
		l->len = end - start + 1;
		return true;
	}
	return false;
}

static bool lex_int(struct lexer *l)
{
	char *str = l->pos;
	int radix = 10;
	int len = 0;
	if (str[len] == '0') {
		radix = 8;
		len++;
		if (str[len] == 'x') {
			radix = 16;
			len++;
		}
	}
	len += digits(str+len, radix);
	if (len <= 0)
		return false;
	// Count length of any u, l, ul, lu, ull, llu
	if (casecmp(str[len], 'U')) {
		len++;
		len += casecmp(str[len], 'L');
		len += casecmp(str[len], 'L');
	} else if (casecmp(str[len], 'L')) {
		len++;
		len += casecmp(str[len], 'L');
		len += casecmp(str[len], 'U');
	}
	l->type = TOK_LIT_INT;
	l->len = len;
	return true;
}

static bool lex_float(struct lexer *l)
{
	char *str = l->pos;
	int len = 0;
	int lhs = 0, dot = 0, rhs = 0, e = 0, exp = 0;
	lhs += digits(str+len, 10);
	len += lhs;
	dot += str[len] == '.';
	len += dot;
	if (!dot)
		return false;
	rhs += digits(str+len, 10);
	len += rhs;
	if (lhs == 0 && rhs == 0)
		return false;
	e += casecmp(str[len], 'e');
	len += e;
	if (e) {
		len += str[len] == '-' || str[len] == '+';
		exp += digits(str+len, 10);
		len += exp;
		if (exp == 0)
			return false;
	}
	if (casecmp(str[len], 'L'))
		len++;
	else if (casecmp(str[len], 'F'))
		len++;
	l->type = TOK_LIT_FLOAT;
	l->len = len;
	return true;
}

/*
 *		Interface functions
 */

void lex_init(struct lexer *l, char *buf, int cap, FILE *src)
{
	l->src = src;
	l->buf = buf;
	l->cap = cap;
	l->pos = buf;
	l->len = 0;
	l->type = TOK_ERROR;
	l->buf[0] = '\0';
}

void lex_next(struct lexer *l)
{
	// Scan to next token position
	l->pos += l->len;
	skip_space(l);
	while (l->pos[0] == '\0') { // EOL
		if (!refill(l)) {
			l->type = TOK_EOF;
			l->len = 0;
			return;
		}
		skip_space(l);
	}
	// Attempt lexing categories
	if (lex_key_op(l))
		return;
	if (lex_ident(l))
		return;
	if (lex_float(l))
		return;
	if (lex_int(l))
		return;
	if (lex_char(l))
		return;
	if (lex_string(l))
		return;
	// Error case: skip characters
	l->type = TOK_ERROR;
	l->len = 1;
}
