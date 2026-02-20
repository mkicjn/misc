//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <limits.h>

char stack[256];

int roll_chr(int chr)
{
	int idx;
	for (idx = 0; idx < sizeof(stack); idx++) {
		int a = stack[0];
		int b = stack[idx];
		stack[0] = b;
		stack[idx] = a;
		if (stack[0] == chr)
			break;
	}
	return idx;
}

int roll_idx(int idx)
{
	for (int i = 0; i < sizeof(stack); i++) {
		int a = stack[0];
		int b = stack[i];
		stack[0] = b;
		stack[i] = a;
		if (i == idx)
			break;
	}
	return stack[0];
}

void encode(void)
{
	while (!feof(stdin)) {
		int chr = getchar();
		if (chr == EOF)
			break;

		int idx = roll_chr(chr);
		putchar(idx);
	}
}

void decode(void)
{
	while (!feof(stdin)) {
		int idx = getchar();
		if (idx == EOF)
			break;

		int chr = roll_idx(idx);
		putchar(chr);
	}
}

int main(int argc, char **argv)
{
	(void)argv;

	for (int i = 0; i < 256; i++)
		stack[i] = i;

	// Compress/decompress based on option
	if (argc > 1)
		decode();
	else
		encode();

	return 0;
}
