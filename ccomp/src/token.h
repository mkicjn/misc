#ifndef TOKENS_H
#define TOKENS_H

/*
 *		Tokens X macro
 *	Ordered so that repeated memcmp in a loop works.
 *	i.e. prefixes come later in the series.
 *	Loop from 0 to TOK_ERROR.
 */
#define TOKENS(X) \
	X(TOK_AUTO, "auto"), \
	X(TOK_BREAK, "break"), \
	X(TOK_CASE, "case"), \
	X(TOK_CHAR, "char"), \
	X(TOK_CONST, "const"), \
	X(TOK_CONTINUE, "continue"), \
	X(TOK_DEFAULT, "default"), \
	X(TOK_DOUBLE, "double"), \
	X(TOK_DO, "do"), \
	X(TOK_ELSE, "else"), \
	X(TOK_ENUM, "enum"), \
	X(TOK_EXTERN, "extern"), \
	X(TOK_FLOAT, "float"), \
	X(TOK_FOR, "for"), \
	X(TOK_GOTO, "goto"), \
	X(TOK_IF, "if"), \
	X(TOK_INLINE, "inline"), \
	X(TOK_INT, "int"), \
	X(TOK_LONG, "long"), \
	X(TOK_REGISTER, "register"), \
	X(TOK_RESTRICT, "restrict"), \
	X(TOK_RETURN, "return"), \
	X(TOK_SHORT, "short"), \
	X(TOK_SIZEOF, "sizeof"), \
	X(TOK_STATIC, "static"), \
	X(TOK_STRUCT, "struct"), \
	X(TOK_SWITCH, "switch"), \
	X(TOK_TYPEDEF, "typedef"), \
	X(TOK_UNION, "union"), \
	X(TOK_UNSIGNED, "unsigned"), \
	X(TOK_SIGNED, "signed"), \
	X(TOK_VOID, "void"), \
	X(TOK_VOLATILE, "volatile"), \
	X(TOK_WHILE, "while"), \
	\
	X(TOK_SET_INC, "++"), \
	X(TOK_SET_DEC, "--"), \
	X(TOK_SET_ADD, "+="), \
	X(TOK_SET_SUB, "-="), \
	X(TOK_SET_MUL, "*="), \
	X(TOK_SET_DIV, "/="), \
	X(TOK_SET_MOD, "%="), \
	X(TOK_SET_LSH, "<<="), \
	X(TOK_SET_RSH, ">>="), \
	X(TOK_SET_AND, "&="), \
	X(TOK_SET_XOR, "^="), \
	X(TOK_SET_IOR, "|="), \
	\
	X(TOK_LSH, "<<"), \
	X(TOK_RSH, ">>"), \
	\
	X(TOK_CMP_EQU, "=="), \
	X(TOK_CMP_NEQ, "!="), \
	X(TOK_CMP_GTE, ">="), \
	X(TOK_CMP_LTE, "<="), \
	\
	X(TOK_CMP_GT, ">"), \
	X(TOK_CMP_LT, "<"), \
	X(TOK_SET_EQU, "="), \
	\
	X(TOK_LOG_OR, "||"), \
	X(TOK_LOG_AND, "&&"), \
	X(TOK_LOG_NOT, "!"), \
	\
	X(TOK_ARROW, "->"), \
	X(TOK_DOT, "."), \
	\
	X(TOK_ADD, "+"), \
	X(TOK_SUB, "-"), \
	X(TOK_MUL, "*"), \
	X(TOK_DIV, "/"), \
	X(TOK_MOD, "%"), \
	\
	X(TOK_BIT_NOT, "~"), \
	X(TOK_BIT_OR, "|"), \
	X(TOK_BIT_XOR, "^"), \
	X(TOK_BIT_AND, "&"), \
	\
	X(TOK_QUESTION, "?"), \
	X(TOK_COLON, ":"), \
	\
	X(TOK_SEMI, ";"), \
	X(TOK_COMMA, ","), \
	X(TOK_LPAREN, "("), \
	X(TOK_RPAREN, ")"), \
	X(TOK_LBRACE, "{"), \
	X(TOK_RBRACE, "}"), \
	X(TOK_LBRACKET, "["), \
	X(TOK_RBRACKET, "]"), \
	\
	X(TOK_ERROR, "Invalid Token"), \
	\
	X(TOK_IDENT, "Identifier"), \
	X(TOK_LIT_FLOAT, "Floating Constant"), \
	X(TOK_LIT_INT, "Integer Constant"), \
	X(TOK_LIT_CHAR, "Character Constant"), \
	X(TOK_LIT_STR, "String"), \
	\
	X(TOK_EOF, "End of File")

#define AS_ENUM(A,B) A
#define AS_STR_ARR(A,B) B
#define AS_LEN_ARR(A,B) (sizeof(B)-1)

enum tok_type {
	TOKENS(AS_ENUM)
};

extern const char *tok_str[];
extern int tok_len[];

#endif
