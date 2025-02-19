#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

uint64_t cbrng(uint64_t n)
{ // Initially inspired by the middle square Weyl sequence and Squares
#ifndef CBRNG_CONST
#define CBRNG_CONST 0xdeadbeef
	// ^ This constant's exact value doesn't even seem to matter much
	//   as long as it's big and not too repetitive in binary.
#endif
	static uint64_t const s = CBRNG_CONST;
	
	uint64_t x = n * s;
	x *= x ^ s;
	// Tacking on some elements from xorshift+ seems to result in strong qualities, even using the full 64-bits
	x ^= x << 13;
	x ^= x >> 7;
	return x + (x >> 32);
}

uint64_t state = 0;
void cbrng_seed(uint64_t s)
{
	state = cbrng(cbrng(s));
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


uint64_t ctr, key;
uint32_t squares(uint64_t ctr, uint64_t key)
{
	uint64_t x = ctr * key;
	uint64_t y = x;
	uint64_t z = x + key;

	x = x*x + y;
	x = (x >> 32) | (x << 32);
	x = x*x + z;
	x = (x >> 32) | (x << 32);
	x = x*x + y;
	x = (x >> 32) | (x << 32);
	x = x*x + z;
	x = (x >> 32) | (x << 32);
	return x;
}
void squares_seed(uint64_t s)
{
	key = s;
	ctr = s;
	key = squares(ctr, key);
	ctr = squares(ctr, key);
}
uint32_t squares_next(void)
{
	ctr += 1;
	return squares(ctr, key);
}


void std_seed(uint64_t s)
{
	srand(s);
}

uint32_t std_next(void)
{
	return rand();
}



void rdrand_seed(uint64_t s)
{
}

uint64_t rdrand_next(void)
{
	uint64_t a;
	asm __volatile__ (
			"rdrand %0"
			: "=r"(a));
	return a;
}



uint64_t xorshift(uint64_t state)
{
	state ^= state << 13;
	state ^= state >> 7;
	return state * CBRNG_CONST;
}

uint64_t xorshift_state = 1;
void xorshift_seed(uint64_t s)
{
	xorshift_state = s;
}
uint64_t xorshift_next_state()
{
	return xorshift(xorshift_state);
}
uint32_t xorshift_next(void)
{
	xorshift_state = xorshift_next_state();
	return xorshift(xorshift_state) >> 32;
}

void xorshift_test(void)
{
	xorshift_seed(0xdeadbeef);
	xorshift_next();
	uint64_t n = 0;
	while (xorshift_state != 0xdeadbeef) {
		xorshift_next();
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
void pcg_seed(uint64_t s)
{
	pcg_state.state = s;
}
uint32_t pcg_next(void)
{
	return pcg32_random_r(&pcg_state);
}



#ifndef RNG
#define RNG cbrng
#endif

#define STR(x) #x
#define XSTR(x) STR(x)
#define RNG_STR XSTR(RNG)

#define CONCAT(x,y) x##y
#define CONCAT2(x,y) CONCAT(x,y)
#define RNGF(x) CONCAT2(RNG,x)

#define rng_seed RNGF(_seed)
#define rng_next RNGF(_next)

void random_pbm(int width, int height, int i)
{
	rng_seed(i);
	uint64_t buf = rng_next();
	int b = sizeof(buf) * 8;
	printf("P1\n%d %d\n", width, height);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			printf("%d ", buf & 1);
			buf >>= 1;
			b--;
			if (b == 0) {
				buf = rng_next();
				b = sizeof(buf) * 8;
			}
		}
		printf("\n");
	}
}

void random_pgm(int width, int height, int i)
{
	rng_seed(i);
	uint32_t buf = rng_next();
	int b = 32;
	printf("P2\n%d %d\n", width, height);
	printf("255\n");
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			printf("%d ", buf & 0xff);
			buf >>= 8;
			b -= 8;
			if (b == 0) {
				buf = rng_next();
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
	fprintf(stderr, "RNG=%s\n", RNG_STR);
	switch (fmt) {
	case RAW:
		if (argc < 3) {
			printf("Not enough arguments\n");
			return 1;
		}
		sscanf(argv[2], "%d", &x);
		if (argc > 3) {
			sscanf(argv[3], "%d", &n);
			rng_seed(n);
		}
		fprintf(stderr, "sizeof(rng_next()): %lu\n", sizeof(rng_next()));
		for (unsigned i = 0; i != x; i++) {
			uint64_t u = rng_next();
			fwrite(&u, sizeof(rng_next()), 1, stdout);
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
			rng_seed(n);
		}
		for (int i = 0; i < y; i++) {
			for (int j = 0; j < x; j++) {
				printf("%d", rng_next());
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
