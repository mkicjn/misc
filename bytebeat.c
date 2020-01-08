#include <stdio.h>
char interpret(char *i, int t)
{
	long stack[100], *s = stack, n = 0, v = 0;
	for (;;) {
		char c = *(i++);
		if (c >= '0' && c <= '9') {
			n = n * 10 + c - '0', v = 1;
			continue;
		} else if (v) {
			*(s++) = n;
			n = 0, v = 0;
		}
		if (!c)
			break;
		switch (c) {
		case 't': *(s++) = t; break;
		case '$': s[0] = s[-1]; s++; break;

		case '<': s--; s[-1] <<= s[0]; break;
		case '>': s--; s[-1] >>= s[0]; break;
		case '+': s--; s[-1]  += s[0]; break;
		case '-': s--; s[-1]  -= s[0]; break;
		case '*': s--; s[-1]  *= s[0]; break;
		case '/': s--; s[-1]  /= s[0]; break;
		case '%': s--; s[-1]  %= s[0]; break;
		case '&': s--; s[-1]  &= s[0]; break;
		case '|': s--; s[-1]  |= s[0]; break;
		case '^': s--; s[-1]  ^= s[0]; break;

		case '"': *(s++) = (long)i; while (*(i++) != '"'); break;
		case ']': s--; s[-1] = ((char *)s[-1])[s[0]];
		}
	}
	return s[-1];
}

int main(int argc, char **argv)
{
	int t = 0;
	if (argc < 2)
		return 1;
	for (;;)
		putchar(interpret(argv[1], t++));
}
