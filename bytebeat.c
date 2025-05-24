//usr/bin/tcc -run $0 $@; exit $?
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// Try this:
// ./a.out 't5*t7>&t3*t10>&|' | pacat --rate=8000

char interpreter(char *s, int t)
{
	// This is dangerously close to a FALSE interpreter...
	intptr_t space[100], *stack = &space[100], n = 0;
	bool n_valid = false;
	for (;;) {
		// Parse numbers character-by-character
		char c = *(s++);
		if ('0' <= c && c <= '9') {
			n = (n * 10) + (c - '0');
			n_valid = true;
			continue;
		} else if (n_valid) {
			*(--stack) = n;
			n_valid = false;
			n = 0;
		}
		// Interpret operations
		switch (c) {
		case '\0':
			return stack[0];
		case 't':
			--stack;
			stack[0] = t;
			break;
		case '$':
			--stack;
			stack[0] = stack[1];
			break;
		case '_':
			stack[0] = -stack[0];
			break;
		case '"':
			--stack;
			stack[0] = (intptr_t)s;
			while (*(s++) != '"')
				if (*s == '\0')
					break;
			break;
		case '@':
			stack[1] = ((char *)stack[1])[stack[0]];
			stack++;
			break;
#define BIN_OP(C, X) \
		case C: \
			stack[1] = (stack[1] X stack[0]); \
			stack++; \
			break;

			BIN_OP('+', +)
			BIN_OP('-', -)
			BIN_OP('*', *)
			BIN_OP('%', %)
			BIN_OP('^', ^)
			BIN_OP('&', &)
			BIN_OP('|', |)
			BIN_OP('<', <<)
			BIN_OP('>', >>)
			BIN_OP('=', ==)
			BIN_OP('!', !=)
#undef BIN_OP
		}

		/*
		// Debug stack printing
		for (int i = 0; i < (&space[100] - stack); i++) {
			fprintf(stderr, "%ld ", stack[i]);
		}
		fprintf(stderr, "\n");
		*/
	}
}

int main(int argc, char **argv)
{
	int t = 0;
	if (argc < 2)
		return 1;
	for (;;)
		putchar(interpreter(argv[1], t++));
}
