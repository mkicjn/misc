#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include "../aterm.h"


/* ******** COUNTER BASED PRNG ******** */

uint32_t cbrng(uint64_t n)
{ // Custom counter-based PRNG
	static uint64_t const s = 0xba2c2cab;

	uint64_t x = n * s;
	x *= x ^ s;
	return x + (x >> 32);
}


/* ******** NOISE GENERATION ******** */

#define VIRT_WIDTH 10000
#define WIDTH 80
#define HEIGHT 24

#define LOD_FAC 8
#define LOD_IMPACT 64

#define SOFT_CLAMP 64

int get_noise(int origin, int x, int y, int smoothing)
{
	// Here's the interesting idea: Procedurally generate in one step with recursion and counter-based RNG
	if (smoothing <= 0) {
		// Soft clamp around map edges
#ifdef SOFT_CLAMP
		if (x <= 0 || x >= WIDTH-1 || y <= 0 || y >= HEIGHT-1)
			return SOFT_CLAMP;
#endif
		// Get high frequency noise
		int noise = cbrng(origin + x + y * VIRT_WIDTH) % 256;
		// Apply low frequency noise
		noise += (cbrng((origin/LOD_FAC) + (x/LOD_FAC) + (y/LOD_FAC) * VIRT_WIDTH) % LOD_IMPACT) - (LOD_IMPACT/2);
		return noise;
	}
	// Calculate average recursively based on smoothing level
	int sum = 0;
	for (int dy = -1; dy <= 1; dy++)
		for (int dx = -1; dx <= 1; dx++)
			sum += get_noise(origin, x + dx, y + dy, smoothing - 1);
	return sum / 9;
}


/* ******** TERRAIN CLASSIFICATION ******** */

enum terrain_type {
	ABYSS,
	OCEAN,
	SHALLOW,
	SAND,
	GRASS,
	WOODS,
	MOUNTAIN,
	PEAK,
	NUM_TERRAIN_TYPES
};

enum terrain_type classify_altitude(int height)
{
#define SEA_LEVEL 128
#define OCEAN_BAND 30
#define SHALLOW_BAND 8
#define SAND_BAND 5
#define GRASS_BAND 10
#define WOODS_BAND 8
#define MOUNTAIN_BAND 12

#define BAND_CASE(type) \
	if (height < type##_BAND) \
		return type; \
	height -= type##_BAND;

	if (height < SEA_LEVEL) {
		height = SEA_LEVEL - height;
		// Water
		BAND_CASE(SHALLOW)
		BAND_CASE(OCEAN)
		return ABYSS;
	} else {
		height -= SEA_LEVEL;
		// Land
		BAND_CASE(SAND)
		BAND_CASE(GRASS)
		BAND_CASE(WOODS)
		BAND_CASE(MOUNTAIN)
		return PEAK;
	}
}


/* ******** TILE DISPLAY ******** */

char tile_char[NUM_TERRAIN_TYPES] = {
	[PEAK] = '^',
	[MOUNTAIN] = '=',
	[WOODS] = '%',
	[GRASS] = '"',
	[SAND] = '~',
	[SHALLOW] = '~',
	[OCEAN] = '~',
	[ABYSS] = ' '
};

const char *tile_color[NUM_TERRAIN_TYPES] = {
	[PEAK] = FG_BCOLR(WHITE),
	[MOUNTAIN] = FG_BCOLR(BLACK),
	[WOODS] = FG_COLR(GREEN),
	[GRASS] = FG_BCOLR(GREEN),
	[SAND] = FG_BCOLR(YELLOW),
	[SHALLOW] = FG_BCOLR(BLUE),
	[OCEAN] = FG_COLR(BLUE),
	[ABYSS] = FG_COLR(BLACK)
};

void print_tile(int height)
{
	enum terrain_type t = classify_altitude(height);
	printf(SGR("%s") "%c", tile_color[t], tile_char[t]);
}


/* ******** WORLD DISPLAY ******** */

void print_depth(int origin, int erosion, int offset)
{
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int depth = get_noise(origin, x, y, erosion);
			print_tile(depth + offset);
		}
		printf(SGR(RESET) "\n\r");
	}
}


/* ******** MAIN AND INPUT LOOP ******** */

int main(int argc, char **argv)
{
	int origin = cbrng(time(NULL));
	char c;
	int depth = 4, offset = 0;
	print_depth(origin, depth, offset);
	if (argc > 1)
		return 0;

	// Notice: no storage of depth map anywhere means area is effectively "infinite" barring virtual width
	// Could probably implement some interesting wrapping mechanism to simulate toroidal or other surfaces
	system("stty raw -echo isig");
	for (;;) {
		printf(ED("2") CUP("1","1"));
		print_depth(origin, depth, offset);
		c = getchar();
		switch (c) {
		case '[':
			depth -= 1;
			break;
		case ']':
			depth += 1;
			break;
		case '-':
			offset -= 1;
			break;
		case '+':
			offset += 1;
			break;
		case 'h':
		case '4':
			origin -= 4;
			break;
		case 'j':
		case '2':
			origin += 4 * VIRT_WIDTH;
			break;
		case 'k':
		case '8':
			origin -= 4 * VIRT_WIDTH;
			break;
		case 'l':
		case '6':
			origin += 4;
			break;
		case 'q':
			system("stty -raw echo");
			return 0;
		}
	}
}
