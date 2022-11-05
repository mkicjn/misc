#include <stdio.h>
#include <stdlib.h>

long collatz(long n)
{
	long i = 0;
	while (n > 1) {
		if (n % 2 == 0)
			n /= 2;
		else
			n = n * 3 + 1;
		i++;
	}
	return i;
}

long max_len(long n)
{
	long m = 0;
	for (long i = 0; i < n; i++) {
		long l = collatz(i);
		if (m < l)
			m = l;
	}
	return m;
}

int main(int argc, char **argv)
{
	printf("%ld\n", max_len(1000000));
}
