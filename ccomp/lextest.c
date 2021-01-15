#include "src/lexer.h"
/*
 *	Just a test driver program for now
 */

int main()
{
	char buf[2048];
	struct lexer l;
	lex_init(&l, buf, sizeof(buf), stdin);

	do {
		lex_next(&l);
		if (l.type >= TOK_ERROR)
			printf("%s: `%.*s`\n", tok_str[l.type], l.len, l.pos);
		else
			printf("Token `%s`: `%.*s`\n", tok_str[l.type], l.len, l.pos);
	} while (l.type != TOK_EOF);

	return 0;
}
