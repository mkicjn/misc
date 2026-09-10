//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

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


// 2D vector operations

struct vec2 {
	double x, y;
};

double vec2dot(struct vec2 *a, struct vec2 *b)
{
	return (a->x * b->x) + (a->y * b->y);
}

void vec2sub(struct vec2 *a, struct vec2 *b)
{
	a->x -= b->x;
	a->y -= b->y;
}


// 2D Perlin noise generation
// (Assuming implementation is correct)

#define VIRT_WIDTH (1ul << 32)

void gradient(uint64_t origin, int x, int y, struct vec2 *g)
{
	double theta = cbrngf(origin + x + y * VIRT_WIDTH) * 2.0 * M_PI;
	g->x = cos(theta);
	g->y = sin(theta);
}

double smoothstep(double x)
{
	return x * x * (3 - 2 * x);
}

double noise(uint64_t origin, int x, int y, unsigned period)
{
	struct vec2 p = {
		.x = ((double)x) / period,
		.y = ((double)y) / period,
	};
	int cx = x / period;
	int cy = y / period;
	double ix = smoothstep(p.x - cx);
	double iy = smoothstep(p.y - cy);
	double noise = 0.0;
	for (int dy = 0; dy <= 1; dy++) {
		for (int dx = 0; dx <= 1; dx++) {
			struct vec2 g;
			gradient(origin, cx + dx, cy + dy, &g);
			struct vec2 dp = {
				.x = cx + dx,
				.y = cy + dy,
			};
			vec2sub(&dp, &p);
			noise += vec2dot(&g, &dp)
				* (dx == 0 ? 1.0 - ix : ix)
				* (dy == 0 ? 1.0 - iy : iy);
		}
	}
	return noise;
}


// Terminal display

#define WIDTH 80
#define HEIGHT 60

#define SAMPLE_PERIOD 16
double sample(int x, int y)
{
	double n = noise(0, x, y, SAMPLE_PERIOD) * (3.0 / 6.0);
	n += noise(0, x, y, SAMPLE_PERIOD / 2) * (2.0 / 6.0);
	n += noise(0, x, y, SAMPLE_PERIOD / 4) * (1.0 / 6.0);

	n = 0.5 + (n * 0.5);
	return n;
}

#define DERATE_WIDTH ((double)(WIDTH/10))
#define DERATE_HEIGHT ((double)(HEIGHT/10))
double edge_derate(int x, int y)
{
	double n = 1.0;

	// Handle nearness to lower limits
	if (x < DERATE_WIDTH)
		n *= (x / DERATE_WIDTH);
	if (y < DERATE_HEIGHT)
		n *= (y / DERATE_HEIGHT);

	// Handle nearness to upper limits
	x = (WIDTH - 1) - x;
	y = (HEIGHT - 1) - y;
	if (x < DERATE_WIDTH)
		n *= (x / DERATE_WIDTH);
	if (y < DERATE_HEIGHT)
		n *= (y / DERATE_HEIGHT);

	// Smooth and weaken the transition
	return 0.50 + (smoothstep(n) * 0.50);
}

const char *shade(double n)
{
	// TODO:
	// * More detail
	// * Reconsider 0.0-1.0 range? (-1.0 to 1.0?)
	// * Compare integers instead?
	// * Add glyph mapping
	//   * "Texturing" by modulus?

	if (n < 0.5) {
		// Water
		if (n > 0.45) {
			return "\033[94;40m"; // Light blue
		} else if (n > 0.25) {
			return "\033[34;40m"; // Blue
		} else {
			return "\033[30;40m"; // Black
		}
	} else {
		// Land
		if (n < 0.525) {
			return "\033[93;40m"; // Yellow
		} else {
			return "\033[92;40m"; // Light green
		}
	}
}

const char *glyph(double n)
{
	if (n < 0.5) {
		// Water
		return "__";
	} else {
		// Land
		if (n < 0.525) {
			return "~~";
		} else if (n < 0.55) {
			switch ((int)(n * 100) % 4) {
			case 0:
				return ",.";
			case 1:
				return ".'";
			case 2:
				return "'\"";
			case 3:
				return "\",";
			default:
				return "  ";
			}
		} else {
			switch ((int)(n * 100) % 5) {
			case 0:
				return ",.";
			case 1:
				return ".'";
			case 2:
				return "'\"";
			case 3:
				return "\",";
			case 4:
				return "%%";
			default:
				return "  ";
			}
		}
	}
}

int main(int argc, char **argv)
{
	// Seed RNG
	if (0 > getrandom(&key, sizeof(key), 0))
		perror("getrandom()");

	// Render
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			double n = sample(x, y);
			n *= edge_derate(x, y);
			printf("%s%s", shade(n), glyph(n));
		}
		printf("\033[m\n");
	}
	return 0;
}
