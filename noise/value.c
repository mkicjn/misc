//usr/bin/env tcc $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <sys/random.h>

// Counter-based RNG

uint64_t splitmix64_ctr(uint64_t key, uint64_t ctr)
{
	uint64_t z = (key + ctr * 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

uint64_t key = 0xdeadbeef;
uint64_t cbrng(uint64_t ctr)
{
	return splitmix64_ctr(key, ctr);
}

double cbrngf(uint64_t ctr)
{
	return (cbrng(ctr) >> 11) * 0x1.0p-53;
}


// Value noise generation
// (Assuming implementation is correct)

#define VIRT_WIDTH (1ul << 32)

double value(uint64_t origin, int x, int y)
{
	return cbrngf(origin + x + y * VIRT_WIDTH);
}

double smoothstep(double x)
{
	return x * x * (3 - 2 * x);
}

int noise(uint64_t origin, int x, int y, unsigned period)
{
	int cx = x / period;
	int cy = y / period;
	double ix = smoothstep((double)x / period - cx);
	double iy = smoothstep((double)y / period - cy);
	double noise = 0.0;
	for (int dy = 0; dy <= 1; dy++) {
		for (int dx = 0; dx <= 1; dx++) {
			double v = value(origin, cx + dx, cy + dy);
			noise += v
				* (dx == 0 ? 1.0 - ix : ix)
				* (dy == 0 ? 1.0 - iy : iy);
		}
	}
	return 256.0 * noise;
}


// Terminal display

#define WIDTH 80
#define HEIGHT 60

int main(int argc, char **argv)
{
	if (0 > getrandom(&key, sizeof(key), 0))
		perror("getrandom()");

	unsigned period = 8;
	if (argc > 1)
		period = atoi(argv[1]);

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int n = noise(0, x, y, period);
			//printf("%d\n", n);
			printf("\033[48;2;%d;%d;%dm  ", n, n, n);
		}
		printf("\033[m\n");
	}
	return 0;
}
