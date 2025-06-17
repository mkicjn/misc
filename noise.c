//$(which tcc) $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Try this:
// ./a.out 100 100 3 2 > out.pgm

uint32_t cbrng(uint64_t n)
{ // Inspired by the middle square Weyl sequence and Squares
	static uint64_t const s = 0xB5AD4ECEDA1CE2A9UL;
	
	uint64_t x = n * s;
	x *= x ^ s;
	return x >> 32;
}

int noise(uint64_t seed, int width, int height, int x0, int y0, int range, int depth)
{
	(void)height;
	uint64_t val_sum = 0;
	uint64_t fac_sum = 0;
#define ABS(X) ((X)<0?-(X):(X))
#define MAX(X,Y) ((X)>(Y)?(X):(Y))
	if (depth <= 0)
		return cbrng(seed + x0 + y0 * width) & 0xff;
	for (int dx = -range; dx <= range; dx++) {
		for (int dy = -range; dy <= range; dy++) {
			int64_t fac = (range*2)-1-ABS(dx)-ABS(dy);
			if (fac <= 0)
				continue;

			int x1 = x0 + dx, y1 = y0 + dy;
			uint64_t val = noise(seed, width, height, x1, y1, range, depth-1);

			val_sum += val * fac;
			fac_sum += fac;
		}
	}
	uint64_t avg = val_sum / fac_sum;
	return avg;
}

void random_pgm(uint64_t seed, int width, int height, int range, int depth)
{
	printf("P2\n%d %d\n", width, height);
	printf("255\n");
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++)
			printf("%d ", noise(seed, width, height, x, y, range, depth));
		printf("\n");
	}
}

int main(int argc, char **argv)
{
	uint64_t width, height, range, depth, seed = 0;
	if (argc < 3) {
		printf("Not enough arguments\n");
		return 1;
	}
	sscanf(argv[1], "%lu", &width);
	sscanf(argv[2], "%lu", &height);
	sscanf(argv[3], "%lu", &range);
	sscanf(argv[4], "%lu", &depth);
	if (argc > 5)
		sscanf(argv[5], "%lu", &seed);
	random_pgm(seed, width, height, range, depth);
	return 0;
}
