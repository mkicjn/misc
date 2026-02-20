//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <unistd.h>
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


// 3D vector operations

struct vec3 {
	double x, y, z;
};

double vec3dot(struct vec3 *a, struct vec3 *b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

void vec3sub(struct vec3 *a, struct vec3 *b)
{
	a->x -= b->x;
	a->y -= b->y;
	a->z -= b->z;
}


// 3D Perlin noise generation
// (Assuming implementation is correct)

#define VIRT_WIDTH (1ul << 20)
#define VIRT_HEIGHT (1ul << 20)
#define COORD(x, y, z) \
	(((z) * VIRT_HEIGHT + (y)) * VIRT_WIDTH + (x))

void gradient(int x, int y, int z, struct vec3 *g)
{
	// https://mathworld.wolfram.com/SpherePointPicking.html
	double theta = cbrngf(COORD(x, y, z) * 2) * 2.0 * M_PI;
	double u = cbrngf(COORD(x, y, z) * 2 + 1) * 2.0 - 1.0;
	g->x = sqrt(1 - u * u) * cos(theta);
	g->y = sqrt(1 - u * u) * sin(theta);
	g->z = u;
}

double smoothstep(double x)
{
	return x * x * (3 - 2 * x);
}

double noise(int x, int y, int z, unsigned period)
{
	struct vec3 p = {
		.x = ((double)x) / period,
		.y = ((double)y) / period,
		.z = ((double)z) / period,
	};
	int cx = x / period;
	int cy = y / period;
	int cz = z / period;
	double ix = smoothstep(p.x - cx);
	double iy = smoothstep(p.y - cy);
	double iz = smoothstep(p.z - cz);
	double noise = 0.0;
	for (int dz = 0; dz <= 1; dz++) {
		for (int dy = 0; dy <= 1; dy++) {
			for (int dx = 0; dx <= 1; dx++) {
				struct vec3 g;
				gradient(cx + dx, cy + dy, cz + dz, &g);
				struct vec3 dp = {
					.x = cx + dx,
					.y = cy + dy,
					.z = cz + dz,
				};
				vec3sub(&dp, &p);
				noise += vec3dot(&g, &dp)
					* (dx == 0 ? 1.0 - ix : ix)
					* (dy == 0 ? 1.0 - iy : iy)
					* (dz == 0 ? 1.0 - iz : iz);
			}
		}
	}
	return noise;
}


// Terminal display

#define WIDTH 80
#define HEIGHT 60
#define DEPTH 65535

#define ANIM_PERIOD 0.05e6

int main(int argc, char **argv)
{
	if (0 > getrandom(&key, sizeof(key), 0))
		perror("getrandom()");

	unsigned period = 16;
	if (argc > 1)
		period = atoi(argv[1]);

	printf("\033[2J");
	for (int z = 0; z < DEPTH; z++) {
		printf("\033[H");
		for (int y = 0; y < HEIGHT; y++) {
			for (int x = 0; x < WIDTH; x++) {
				double n = noise(x, y, z, period);
				n += noise(x, y, z, period / 2) / 2.0;
				n += noise(x, y, z, period / 4) / 4.0;
				//printf("%d\n", n);
				int i = 128 + n * 128;
				printf("\033[48;2;%d;%d;%dm  ", i, i, i);
			}
			printf("\033[m\n");
		}
		usleep(ANIM_PERIOD);
	}
	return 0;
}
