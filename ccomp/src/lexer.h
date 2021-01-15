#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "token.h"

struct lexer {
	FILE *src; // Input stream source
	char *buf; // Buffer location
	// ^ Buffer should be as long as the max string length (- 2)
	char *pos; // Position of current token
	int cap; // Buffer capacity
	int len; // Length of current token
	enum tok_type type; // Type of current token
	// ^ See token.h
};

void lex_init(struct lexer *l, char *buf, int cap, FILE *src);
/*
 * lex_init initializes a lexer struct at the address pointed to by l.
 * Sets all fields in struct lexer and places an empty string in buf.
 */

void lex_next(struct lexer *l);
/*
 * lex_next advances lexical analysis by one token.
 * Updates l->pos, l->len, and l->type to reflect the next token.
 * Previous values obtained this way are not necessarily preserved.
 */

#endif


