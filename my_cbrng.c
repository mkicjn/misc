#include <stdio.h>
#include <stdint.h>

uint32_t cbrng(int n)
{ // Inspired by the middle square Weyl sequence and Squares
	static uint64_t const s = 0xB5AD4ECEDA1CE2A9UL;
	
	uint64_t x = n * s;
	x *= x ^ s;
	return x >> 32;
}

int main(int argc, char **argv)
{
	// Generates argv[1] numbers and outputs binary to stdout.
	// I made this for use with the Diehard tests.
	int n = 0;
	if (argc < 2)
		return 1;
	sscanf(argv[1], "%d", &n);
	for (int i = 0; i < n; i++) {
		unsigned u = cbrng(i);
		fwrite(&u, sizeof(unsigned), 1, stdout);
	}
	return 0;
}
