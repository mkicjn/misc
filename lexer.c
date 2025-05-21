// A tiny reusable lexer that might be used for something eventually
// TODO: Try to produce a similar thing for recursive descent parsing

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Token types: enum, match function, and description
// NOTE: The order of the list may affect token recognition
#define FOREACH_TOK_TYPE(X) \
	X(TOK_EOF, lex_eof, "end of file") \
	X(TOK_SPACE, lex_space, "whitespace") \
	X(TOK_WORD, lex_word, "identifier") \
	X(TOK_NUMBER, lex_number, "integer") \
	X(TOK_ERROR, lex_error, "error")

// Declare enum values based on the token types
enum tok_type {
#define LIST_ENUM(ENUM, FN, DESC) ENUM,
	FOREACH_TOK_TYPE(LIST_ENUM)
};

// Declare lexer functions ahead of time
#define DECL_LEX_FN(ENUM, FN, DESC) \
	static bool FN(void);
FOREACH_TOK_TYPE(DECL_LEX_FN)

// Function for calling each match function until one succeeds
// NOTE: One of these should always succeed (e.g., have a catch-all error token at the end)
static void try_matches(void)
{
#define TRY_LEX_FN(ENUM, FN, DESC) \
		if (FN()) \
			return;
	FOREACH_TOK_TYPE(TRY_LEX_FN)
}


// Internal lexer variables
#define MAX_TOK_LEN 1024
static int buf[MAX_TOK_LEN];
static size_t buf_len = 0;

// External lexer variables
FILE *token_source = NULL;
char token[MAX_TOK_LEN];
size_t token_len = 0;
enum tok_type token_type = TOK_ERROR;


// Buffer management, the simplest possible way
static void refill(void)
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
}

// Lexer driver - refill buffers and run match functions
enum tok_type token_next(void)
{
	refill();
	try_matches();
	return token_type;
}


////////////////////////////////////////////////////////////////////////////////
// Match functions
// NOTE: All these have to do is check buf (or token) and update token_type and token_len on match

// TODO: Simplify using "predicates" like sequence_of() and exact_match()

static bool lex_eof(void)
{
	if (buf[0] == EOF) {
		token_type = TOK_EOF;
		token_len = 0;
		return true;
	}
	return false;
}

static bool lex_error(void)
{
	token_type = TOK_ERROR;
	token_len = 1; // Skip problematic characters
	return true;
}

static bool lex_space(void)
{
	size_t len;
	for (len = 0; len < MAX_TOK_LEN; len++) {
		int c = buf[len];
		if ('\0' <= c && c <= ' ')
			continue;
		break;
	}
	if (len > 0) {
		token_type = TOK_SPACE;
		token_len = len;
		return true;
	}
	return false;
}

static bool lex_word(void)
{
	size_t len = 0;
	for (len = 0; len < MAX_TOK_LEN; len++) {
		int c = buf[len];
		if ('0' <= c && c <= '9' && len > 0)
			continue;
		if ('A' <= c && c <= 'Z')
			continue;
		if ('_' == c)
			continue;
		if ('a' <= c && c <= 'z')
			continue;
		break;
	}
	if (len > 0) {
		token_type = TOK_WORD;
		token_len = len;
		return true;
	}
	return false;
}

static bool lex_number(void)
{
	size_t len = 0;
	for (len = 0; len < MAX_TOK_LEN; len++) {
		int c = buf[len];
		if ('0' <= c && c <= '9')
			continue;
		break;
	}
	if (len > 0) {
		token_type = TOK_NUMBER;
		token_len = len;
		return true;
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////
// Test program

// Generate a table of descriptions for each token type
const char *tok_desc[] = {
#define LIST_DESC(ENUM, FN, DESC) DESC,
	FOREACH_TOK_TYPE(LIST_DESC)
};

int main()
{
	token_source = stdin;
	while (token_next() != TOK_EOF) {
		printf("%s: %.*s\n", tok_desc[token_type], (int)token_len, token);
	}
}
