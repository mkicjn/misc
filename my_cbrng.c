#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint32_t cbrng(uint64_t n)
{ // Inspired by the middle square Weyl sequence and Squares
	static uint64_t const s = 0xB5AD4ECEDA1CE2A9UL;
	
	uint64_t x = n * s;
	x *= x ^ s;
	return x >> 32;
}

uint64_t state = 0;
uint32_t cbrng_seed(uint64_t s)
{
	state = cbrng(s);
}
uint64_t cbrng_next_state()
{
	return state+1;
}
uint32_t cbrng_next(void)
{
	state = cbrng_next_state();
	return cbrng(state);
}



uint32_t lfsr(uint64_t state)
{
	state ^= state << 1;
	state ^= state >> 3;
	state ^= state << 10;
	return state;
}

uint64_t lfsr_state = 1;
uint32_t lfsr_seed(uint64_t s)
{
	lfsr_state = s;
}
uint64_t lfsr_next_state()
{
	return lfsr(lfsr_state);
}
uint32_t lfsr_next(void)
{
	lfsr_state = lfsr_next_state();
	return lfsr(lfsr_state);
}

void lfsr_test(void)
{
	lfsr_seed(0xdeadbeef);
	lfsr_next();
	uint64_t n = 0;
	while (lfsr_state != 0xdeadbeef) {
		lfsr_next();
		n++;
	}
	printf("deadbeef cycle: %lu\n", n);
}



#define PCG32_INITIALIZER   { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL }
typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;

uint32_t pcg32_random_r(pcg32_random_t* rng)
{
	uint64_t oldstate = rng->state;
	rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
	uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

pcg32_random_t pcg_state = PCG32_INITIALIZER;
uint32_t pcg_seed(uint64_t s)
{
	pcg_state.state = s;
}
uint32_t pcg_next(void)
{
	return pcg32_random_r(&pcg_state);
}


// From here, can do :.,$s/cbrng/pcg/g etc. to change which algorithm is used.
// TODO: Make this a compile time option, or something like that.

void random_pbm(int width, int height, int i)
{
	cbrng_seed(i);
	uint32_t buf = cbrng_next();
	int b = 32;
	printf("P1\n%d %d\n", width, height);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			printf("%d ", buf & 1);
			buf >>= 1;
			b--;
			if (b == 0) {
				buf = cbrng_next();
				b = 32;
			}
		}
		printf("\n");
	}
}

void random_pgm(int width, int height, int i)
{
	cbrng_seed(i);
	uint32_t buf = cbrng_next();
	int b = 32;
	printf("P2\n%d %d\n", width, height);
	printf("255\n");
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			printf("%d ", buf & 0xff);
			buf >>= 8;
			b -= 8;
			if (b == 0) {
				buf = cbrng_next();
				b = 32;
			}
		}
		printf("\n");
	}
}

#include <time.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	enum {RAW = 1, CSV, PBM, PGM} fmt = 0;

	if (argc < 2) {
		printf("No selection\n");
		return 1;
	}
	if (!strcasecmp(argv[1], "raw"))
		fmt = RAW;
	if (!strcasecmp(argv[1], "csv"))
		fmt = CSV;
	if (!strcasecmp(argv[1], "pbm"))
		fmt = PBM;
	if (!strcasecmp(argv[1], "pgm"))
		fmt = PGM;

	int x, y, n = 0;
	switch (fmt) {
	case RAW:
		if (argc < 3) {
			printf("Not enough arguments\n");
			return 1;
		}
		sscanf(argv[2], "%d", &x);
		if (argc > 3) {
			sscanf(argv[3], "%d", &n);
			cbrng_seed(n);
		}
		for (unsigned i = 0; i != x; i++) {
			unsigned u = cbrng_next();
			fwrite(&u, sizeof(unsigned), 1, stdout);
		}
		break;
	case CSV:
		if (argc < 4) {
			printf("Not enough arguments\n");
			return 1;
		}
		sscanf(argv[2], "%d", &x);
		sscanf(argv[3], "%d", &y);
		if (argc > 4) {
			sscanf(argv[4], "%d", &n);
			cbrng_seed(n);
		}
		for (int i = 0; i < y; i++) {
			for (int j = 0; j < x; j++) {
				printf("%d", cbrng_next());
				if (j < x - 1)
					putchar(',');
			}
			putchar('\n');
		}
		break;
	case PBM:
		if (argc < 4) {
			printf("Not enough arguments\n");
			return 1;
		}
		sscanf(argv[2], "%d", &x);
		sscanf(argv[3], "%d", &y);
		if (argc > 4)
			sscanf(argv[4], "%d", &n);
		random_pbm(x, y, n);
		break;
	case PGM:
		if (argc < 4) {
			printf("Not enough arguments\n");
			return 1;
		}
		sscanf(argv[2], "%d", &x);
		sscanf(argv[3], "%d", &y);
		if (argc > 4)
			sscanf(argv[4], "%d", &n);
		random_pgm(x, y, n);
		break;
	default:
		printf("Invalid selection\n");
		return 1;
	}
	return 0;
}
