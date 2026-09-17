//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include <sys/random.h>

// Counter-based RNG

uint64_t cbrng(uint64_t key, uint64_t ctr)
{
	// https://prng.di.unimi.it/splitmix64.c
	uint64_t z = (key + ctr * 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

double cbrngf(uint64_t key, uint64_t ctr)
{
	// https://prng.di.unimi.it/
	// "Generating uniform doubles in the unit interval"
	return (cbrng(key, ctr) >> 11) * 0x1.0p-53;
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


// 3D gradient noise

#define VIRT_WIDTH (1ul << 20)
#define VIRT_HEIGHT (1ul << 20)

void gradient(uint64_t key, int x, int y, int z, struct vec3 *g)
{
	uint64_t p = (z * VIRT_HEIGHT + y) * VIRT_WIDTH + x;
	// https://mathworld.wolfram.com/SpherePointPicking.html
	double theta = cbrngf(key, p * 2) * 2.0 * M_PI;
	double u = cbrngf(key, p * 2 + 1) * 2.0 - 1.0;
	g->x = sqrt(1 - u * u) * cos(theta);
	g->y = sqrt(1 - u * u) * sin(theta);
	g->z = u;
}

double smoothstep(double x)
{
	return x * x * (3 - 2 * x);
}

double noise(uint64_t key, int x, int y, int z, unsigned period)
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
				gradient(key, cx + dx, cy + dy, cz + dz, &g);
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

#define WIDTH 72
#define HEIGHT 60

#define DERATE_WIDTH ((double)(WIDTH/10))
#define DERATE_HEIGHT ((double)(HEIGHT/10))
double surface_bias(int x, int y)
{
	double n = 1.0;

	// Trend towards 0 at west/north edges
	if (x < DERATE_WIDTH)
		n *= (x / DERATE_WIDTH);
	if (y < DERATE_HEIGHT)
		n *= (y / DERATE_HEIGHT);

	// Trend towards 0 at east/south edges
	x = (WIDTH - 1) - x;
	y = (HEIGHT - 1) - y;
	if (x < DERATE_WIDTH)
		n *= (x / DERATE_WIDTH);
	if (y < DERATE_HEIGHT)
		n *= (y / DERATE_HEIGHT);

	// Smooth and weaken transition
	return 0.5 + (smoothstep(n) * 0.5);
}

#define SAMPLE_PERIOD 20
double surface_sample(uint64_t key, int x, int y)
{
	// Fractal noise base
	double surface = 0.0;
	surface += noise(key,     x, y, 0, SAMPLE_PERIOD)     * (3.0 / 6.0);
	surface += noise(key + 1, x, y, 0, SAMPLE_PERIOD / 2) * (2.0 / 6.0);
	surface += noise(key + 2, x, y, 0, SAMPLE_PERIOD / 4) * (1.0 / 6.0);

	// Add feature layers (rivers & ridges)
	double river = noise(key + 3, x, y, 0, SAMPLE_PERIOD / 2);
	if (-0.1 <= river && river <= 0.1)
		surface -= 3 * (0.1 - fabs(river));

	double ridge = noise(key + 4, x, y, 0, SAMPLE_PERIOD);
	if (-0.1 <= ridge && ridge <= 0.1)
		surface += 2 * (0.1 - fabs(ridge));

	// Trend towards sea level at edges
	surface = -1.0 + (1.0 + surface) * surface_bias(x, y);

	return surface;
}

const char *plains_texture(double n)
{
	static const char *palette[] = {
		"\033[0;92;40m" ",.",
		"\033[0;92;40m" ".'",
		"\033[0;92;40m" "'\"",
		"\033[0;92;40m" "\",",
	};
	int i = (int)(n * 10000);
	return palette[i % 4];
}

const char *forest_texture(double n)
{
	static const char *palette[] = {
		"\033[0;2;32;40m" "%%",
		"\033[0;2;32;40m" ".%",
		"\033[0;2;32;40m" "%,",
		"\033[0;2;32;40m" "'%",
		"\033[0;2;32;40m" "%\"",
		"\033[0;2;32;40m" ",.",
		"\033[0;2;32;40m" ".'",
		"\033[0;2;32;40m" "'\"",
		"\033[0;2;32;40m" "\",",
	};
	int i = (int)(n * 10000);
	if (i % 5 == 0) {
		return palette[i % 5];
	} else {
		return palette[5 + i % 4];
	}
}

const char *surface_shade(double n)
{
	if (n < 0.0) {
		// Water
		if (n > -0.10) {
			return "\033[0;94;40m" "~~"; // Light blue
		} else if (n > -0.40) {
			return "\033[0;34;40m" "~~"; // Blue
		} else {
			return "\033[0;30;40m" "~~"; // Black
		}
	} else {
		// Land
		if (n < 0.05) {
			return "\033[0;93;40m" "~~"; // Yellow
		} else if (n < 0.15) {
			return plains_texture(n); // Bright green
		} else if (n < 0.25) {
			return forest_texture(n); // Dark green
		} else if (n < 0.30) {
			return "\033[0;2;37;40m" "=="; // Dark gray
		} else if (n < 0.40) {
			return "\033[0;37;40m" "^^"; // White
		} else {
			return "\033[0;1;97;40m" "^^"; // Bright white
		}
	}
}

void visualize_surface(uint64_t key)
{
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			double n = surface_sample(key, x, y);
			int i = 128 + n * 128;
			printf("\033[48;2;%d;%d;%dm  ", i, i, i);
		}
		printf("\033[m\n");
	}
}

void render_world(uint64_t key)
{
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			double n = surface_sample(key, x, y);
			printf("%s", surface_shade(n));
		}
		printf("\033[m\n");
	}
}

int main(int argc, char **argv)
{
	uint64_t key = 0xbad5eed;
	bool random_seed = true;
	bool xray_mode = false;

	for (int i = 0; i < argc; i++) {
		switch (argv[i][0]) {
		case 'k':
			sscanf(&argv[i][1], "%lx", &key);
			random_seed = false;
			break;
		case 'x':
			xray_mode = true;
			break;
		}
	}

	if (random_seed) {
		if (0 > getrandom(&key, sizeof(key), 0))
			perror("getrandom()");
	}

	if (xray_mode) {
		visualize_surface(key);
	} else {
		render_world(key);
	}

	printf("Seed: %lx\n", key);
	return 0;
}
