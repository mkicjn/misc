//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

bool seq_prime(const unsigned long n)
{
	static int n_primes = 0, cap = 0;
	static int *primes = NULL;
	if (primes == NULL) {
		primes = malloc(1 * sizeof(int));
		cap = 1;
		primes[n_primes++] = 2;
	}
	if (n < 2)
		return false;
	int lim = (int)sqrt((double)n);
	for (int i = 0; primes[i] <= lim; i++)
		if (n % primes[i] == 0)
			return false;
	if (n_primes+1 > cap) {
		cap *= 2;
		primes = realloc(primes, cap * sizeof(int));
	}
	primes[n_primes++] = n;
	return true;
}

char *ordinal_suffix(int n)
{
	switch (n % 10) {
	case 1: return "st";
	case 2: return "nd";
	case 3: return "rd";
	default: break;
	}
	return "th";
}

int main(int argc, char **argv)
{
	int n = 0;
	int lim = 2147483647;
	if (argc > 1)
		lim = atoi(argv[1]);
	for (unsigned long i = 0; i < lim; i++) {
		if (seq_prime(i)) {
			n++;
			printf("%d%s prime: %lu\n", n, ordinal_suffix(n), i);
		}
	}
	return 0;
}
