//`which tcc` $CFLAGS -run $0 "$@"; exit $?
// A tiny lexer framework that might be useful for something eventually
// TODO: Try to produce a similar sort of thing for recursive descent parsing / ASTs

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Character classes
#define WHITESPACE_CHARS \
	"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10" \
	"\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20"

#define LETTER_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define NUMBER_CHARS "0123456789"
#define WORD_CHARS   "_" LETTER_CHARS NUMBER_CHARS

// List of all token types: enum name, description, match function, and arguments (if any)
// NOTE: The order of tokens in this list may affect token recognition
#define FOREACH_TOK_TYPE(X) \
	X(TOK_EOF, "end of file", lex_eof) \
	X(TOK_COMMENT, "comment", lex_comment) \
	X(TOK_STRING, "string literal", lex_string) \
	X(TOK_CHAR, "character literal", lex_char) \
	X(TOK_SPACE, "whitespace", sequence_of, WHITESPACE_CHARS) \
	X(TOK_NUMBER, "integer", sequence_of, NUMBER_CHARS) \
	X(TOK_WORD, "identifier", sequence_of, WORD_CHARS) \
	X(TOK_INC, "increment", exact_match, "++") \
	X(TOK_PLUS, "plus sign", exact_match, "+") \
	X(TOK_ARROW, "right arrow", exact_match, "->") \
	X(TOK_DEC, "decrement", exact_match, "--") \
	X(TOK_MINUS, "minus sign", exact_match, "-") \
	X(TOK_STAR, "asterisk", exact_match, "*") \
	X(TOK_FSLASH, "forward slash", exact_match, "/") \
	X(TOK_PERCENT, "percent sign", exact_match, "%") \
	X(TOK_AMPAMP, "logical and", exact_match, "&&") \
	X(TOK_AMP, "bitwise and", exact_match, "&") \
	X(TOK_BARBAR, "logical or", exact_match, "||") \
	X(TOK_BAR, "bitwise or", exact_match, "|") \
	X(TOK_CARET, "exclusive or", exact_match, "^") \
	X(TOK_LTEQ, "less-than-or-equal-to", exact_match, "<=") \
	X(TOK_LT, "less-than", exact_match, "<") \
	X(TOK_GTEQ, "greater-than-or-equal-to", exact_match, ">=") \
	X(TOK_GT, "greater-than", exact_match, ">") \
	X(TOK_EQEQ, "equal-to", exact_match, "==") \
	X(TOK_EQ, "assignment", exact_match, "=") \
	X(TOK_SEMICOL, "semicolon", exact_match, ";") \
	X(TOK_QMARK, "question mark", exact_match, "?") \
	X(TOK_EXCLAIM, "exclamation point", exact_match, "!") \
	X(TOK_COLON, "colon", exact_match, ":") \
	X(TOK_ELLIPSE, "ellipse", exact_match, "...") \
	X(TOK_DOT, "period", exact_match, ".") \
	X(TOK_COMMA, "comma", exact_match, ",") \
	X(TOK_LPAREN, "left parenthesis", exact_match, "(") \
	X(TOK_RPAREN, "right parenthesis", exact_match, ")") \
	X(TOK_LBRACK, "left bracket", exact_match, "[") \
	X(TOK_RBRACK, "right bracket", exact_match, "]") \
	X(TOK_LBRACE, "left brace", exact_match, "{") \
	X(TOK_RBRACE, "right brace", exact_match, "}") \
	X(TOK_ERROR, "unknown", lex_error)

// Declare enum values based on list of token types
enum tok_type {
#define LIST_ENUM(ENUM, DESC, FN, ...) ENUM,
	FOREACH_TOK_TYPE(LIST_ENUM)
};

// Generate a table of descriptions for each token type
const char *tok_desc[] = {
#define LIST_DESC(ENUM, DESC, FN, ...) DESC,
	FOREACH_TOK_TYPE(LIST_DESC)
};

// Internal lexer variables
#define MAX_TOK_LEN 4096
static int buf[MAX_TOK_LEN];
static size_t buf_len = 0;

// External lexer variables
FILE *token_source = NULL;
char token[MAX_TOK_LEN];
size_t token_len = 0;
enum tok_type token_type = TOK_ERROR;


////////////////////////////////////////////////////////////////////////////////
// Match functions

// General cases

static size_t sequence_of(const char *chrs)
{
	size_t len;
	for (len = 0; len < MAX_TOK_LEN; len++) {
		if (buf[len] == EOF)
			break;
		if (strchr(chrs, token[len]) == NULL)
			break;
	}
	return len;
}

static size_t exact_match(const char *str)
{
	size_t len = strlen(str);
	if (strncmp(token, str, len) == 0)
		return len;
	return 0;
}

static size_t find_end(size_t whence, const char *str)
{
	size_t len = strlen(str);
	for (size_t i = whence; i < MAX_TOK_LEN; i++) {
		if (buf[i + len - 1] == EOF) {
			return 0;
		} else if (buf[i] == '\\') {
			i++; // Skip escaped characters
		} else if (strncmp(&token[i], str, len) == 0) {
			return i + len;
		}
	}
	return 0;
}

// Special cases

static size_t lex_eof(void)
{
	return (buf[0] == EOF);
}

static size_t lex_error(void)
{
	return 1; // Skip problem characters
}

static size_t lex_string(void)
{
	if (exact_match("\""))
		return find_end(1, "\"");
	return 0;
}

static size_t lex_char(void)
{
	if (exact_match("'"))
		return find_end(1, "'");
	return 0;
}

static size_t lex_comment(void)
{
	if (exact_match("//") || exact_match("#"))
		return find_end(1, "\n");
	if (exact_match("/*"))
		return find_end(2, "*/");
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Lexer driver - refill buffers and run match functions

void token_next(void)
{
	// Slide everything over the last token
	memmove(&token[0], &token[token_len], (MAX_TOK_LEN - token_len) * sizeof(char));
	memmove(&buf[0], &buf[token_len], (MAX_TOK_LEN - token_len) * sizeof(int));
	buf_len -= token_len;
	// Refill both buffers until they're full
	while (buf_len < MAX_TOK_LEN) {
		int c = fgetc(token_source);
		buf[buf_len] = c;
		token[buf_len] = (char)c;
		buf_len++;
	}

	// Try all matching functions until one succeeds
	// NOTE: One of these _must_ succeed, even if it's just a catch-all error token
#define TRY_MATCH_FN(ENUM, DESC, FN, ...) \
		token_type = ENUM; \
		token_len = FN(__VA_ARGS__); \
		if (token_len > 0) \
			return;
	FOREACH_TOK_TYPE(TRY_MATCH_FN)
}


////////////////////////////////////////////////////////////////////////////////
// Test program

#ifndef NO_LEX_MAIN
int main()
{
	token_source = stdin;
	for (;;) {
		token_next();
		if (token_type == TOK_EOF)
			break;
		if (token_type == TOK_SPACE)
			continue;
		if (token_type == TOK_COMMENT)
			continue;
		printf("%s: %.*s\n", tok_desc[token_type], (int)token_len, token);
	}
	return 0;
}
#endif
