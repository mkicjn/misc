//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

// This code tries to encode a ternary string in binary arithmetically
// It has some inefficiency due to word boundaries, and thus the size ratio
// is approximately LOG2(3) + (2 / WIDTH)

int main()
{
	uint64_t n = 1;
	while (!feof(stdin)) {
		int c = getchar();
		if (c == EOF)
			break;
		if (!('0' <= c && c <= '2'))
			continue;
		int d = c - '0';
		uint64_t m = (n << 1) + n + d;
		if (m > UINT32_MAX) {
			printf("%lb", n);
			n = 1;
			n = (n << 1) + n + d;
		} else {
			n = m;
		}
	}
	printf("%lb\n", n);
}
