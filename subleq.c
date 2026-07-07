//`which tcc` $CFLAGS -run $0 $@; exit $?
#include <stdio.h>

int hello[] = {
	                    // static char str[] = "Hello, world!"
	[17] = 'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\n', '\0',
	 [0] = 15, 17, -1,  // static char *p1 = str; if (*p1 == '\0') halt;
	 [3] = 17, -1, -1,  // static char *p2 = str; putchar(p2);
	 [6] = 16, 1, -1,   // p1++;
	 [9] = 16, 3, -1,   // p2++;
	[12] = 15, 15, 0,   // goto 0;
	[15] = 0,
	[16] = -1,
};

int mem[1 << 12];

int main(int argc, char **argv)
{
	// Load program
	if (argc > 1) {
		FILE *f = fopen(argv[1], "r");
		if (!f)
			return 1;
		int p = 0;
		while (fscanf(f, "%d", &mem[p]) > 0)
			p++;
		fclose(f);
	} else {
		for (int i = 0; i < sizeof(hello) / sizeof(hello[0]); i++)
			mem[i] = hello[i];
	}

	// Virtual machine
	int p = 0;
	while (p >= 0) {
		int a = mem[p++];
		int b = mem[p++];
		int c = mem[p++];

		if (a < 0) {
			mem[b] = getchar();
		} else if (b < 0) {
			putchar(mem[a]);
		} else {
			mem[b] -= mem[a];
			if (mem[b] <= 0)
				p = c;
		}
	}

	return 0;
}
