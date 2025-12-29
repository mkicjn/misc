//$(which tcc) $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>

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

void gradient(int x, int y, struct vec2 *g)
{
	// Pseudo-randomly generate a randomly-oriented unit vector
	double theta = cbrngf(x + y * VIRT_WIDTH) * 2.0 * M_PI;
	g->x = cos(theta);
	g->y = sin(theta);
}

double smoothstep(double x)
{
	return x * x * (3 - 2 * x);
}

double noise(int x, int y, double period)
{
	// Scale input coordinate according to noise period
	struct vec2 p = {
		.x = ((double)x) / period,
		.y = ((double)y) / period,
	};
	// Find cell origin as floor of input coordinates
	int cx = floor(p.x);
	int cy = floor(p.y);
	// Pre-calculate interpolation factor for cell origin (smoothed via smoothstep)
	double ix = smoothstep(p.x - cx);
	double iy = smoothstep(p.y - cy);
	// For each vertex of the current cell:
	double noise = 0.0;
	for (int dy = 0; dy <= 1; dy++) {
		for (int dx = 0; dx <= 1; dx++) {
			// Determine the gradient vector at the current vertex
			struct vec2 g;
			gradient(cx + dx, cy + dy, &g);
			// Calculate the difference vector from the input point to the current vertex
			struct vec2 dp = {
				.x = cx + dx,
				.y = cy + dy,
			};
			vec2sub(&dp, &p);
			// Calculate the dot product between these, interpolate the result, and add to total
			noise += vec2dot(&g, &dp)
				* (dx == 0 ? 1.0 - ix : ix)
				* (dy == 0 ? 1.0 - iy : iy);
		}
	}
	// Return total
	return noise;
}


// Terminal display

static inline double interpolate(double f, double a, double b)
{
	return ((1.0 - f) * a) + (f * b);
}

static inline bool range_normalize(double *n, double lo, double hi)
{
	double m = (*n - lo) / (hi - lo);
	if (0.0 <= m && m <= 1.0) {
		*n = m;
		return true;
	}
	return false;
}

void shade_px(double n, uint8_t *r, uint8_t *g, uint8_t *b)
{
	if (!range_normalize(&n, -1.0, 1.0)) {
		// Domain error --> magenta
		*r = 255;
		*g = 0;
		*b = 255;
	}

	double thresh = 0.35;
	if (range_normalize(&n, 0.0, thresh)) {
		*r = interpolate(n,  96, 255);
		*g = interpolate(n, 128, 255);
		*b = interpolate(n, 192, 255);
	} else if (range_normalize(&n, thresh, 1.0)) {
		*r = interpolate(n, 255, 128);
		*g = interpolate(n, 255, 128);
		*b = interpolate(n, 255, 128);
	}
}

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

int screen_width = 160;
int screen_height = 60;
void screen(int x, int y, double period)
{
	printf("\033[1;1H");
	for (int dy = 0; dy < screen_height - 1; dy++) {
		for (int dx = 0; dx < screen_width / 2; dx++) {
			double n = noise(x + dx, y + dy, period);
			n += noise(x + dx, y + dy, period * 2.0) * 2.0;
			n += noise(x + dx, y + dy, period / 2.0) / 2.0;
			n += noise(x + dx, y + dy, period / 4.0) / 4.0;
			n = CLAMP(n, -1.0, 1.0);

			uint8_t r, g, b;
			shade_px(n, &r, &g, &b);
			printf("\033[48;2;%d;%d;%dm  ", r, g, b);
		}
		printf("\033[m\r\n");
	}
}

void get_term_size(int *rows, int *cols)
{
	printf("\033[999;999H");
	printf("\033[6n");
	fflush(stdout);
	scanf("\033[%d;%dR", rows, cols);
}

void sig_handler(int signo)
{
	switch (signo) {
	case SIGWINCH:
		get_term_size(&screen_height, &screen_width);
		break;
	default:
		system("stty sane");
		exit(0);
		break;
	}
}

int main(int argc, char **argv)
{
	if (0 > getrandom(&key, sizeof(key), 0))
		perror("getrandom()");

	double period = 20;
	if (argc > 1)
		period = atoi(argv[1]);

	printf("\033[2J");

#ifdef SHOW_SHADE_RANGE
	printf("\033[%d;1H", HEIGHT + 2);
	for (int x = 0; x < WIDTH; x++) {
		double n = (2.0 * x / WIDTH) - 1.0;
		uint8_t r, g, b;
		shade_px(n, &r, &g, &b);
		printf("\033[48;2;%d;%d;%dm  ", r, g, b);
	}
#endif

	if (isatty(fileno(stdin))) {
		signal(SIGINT, sig_handler);
		signal(SIGWINCH, sig_handler);
		system("stty raw -echo isig");
		raise(SIGWINCH);
	}

	int x = 0, y = 0;
	for (;;) {
		screen(x, y, period);
		switch (getchar()) {
		case '{':
			key--;
			break;
		case '}':
			key++;
			break;
		case '[':
			period--;
			break;
		case ']':
			period++;
			break;
		case 'h':
			x--;
			break;
		case 'j':
			y++;
			break;
		case 'k':
			y--;
			break;
		case 'l':
			x++;
			break;
		case 'q':
			raise(SIGINT);
			break;
		}
	}
	return 0;
}
