//`which tcc` $CFLAGS -run $0 "$@"; exit $?
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

#define VIRTUAL_WIDTH 100000
#define WIDTH 150
#define HEIGHT 50

#define BORDER_ELEVATION_VAL 64

#define NORTH_POLE_TEMP_MOD 40
#define EQUATOR_TEMP_MOD 20
#define CLIMATE_BAND_HEIGHT (HEIGHT/8)

#define LOD_FAC ((WIDTH*HEIGHT)/500)
#define LOD_IMPACT 32

int smooth_avg(int (*func)(int origin, int x, int y), int origin, int x, int y, int smoothing)
{
	// Here's the interesting idea: Procedurally generate in one step with recursion and counter-based RNG
	if (smoothing <= 0) {
		return func(origin, x, y);
	}
	// Calculate average recursively based on smoothing level
	int sum = 0;
	for (int dy = -1; dy <= 1; dy++)
		for (int dx = -1; dx <= 1; dx++)
			sum += smooth_avg(func, origin, x + dx, y + dy, smoothing - 1);
	return sum / 9;
}


int elevation_noise(int origin, int x, int y)
{
	// Soft clamp around map edges
	if (x <= 0 || x >= WIDTH-1 || y <= 0 || y >= HEIGHT-1)
		return BORDER_ELEVATION_VAL;

	// Get high frequency noise
	int noise;
	noise = cbrng(origin + x + y * VIRTUAL_WIDTH) % 256;

	// Apply low frequency noise
	noise += (cbrng((origin/LOD_FAC) + (x/LOD_FAC) + (y/LOD_FAC) * VIRTUAL_WIDTH) % LOD_IMPACT) - (LOD_IMPACT/2);

	return noise;
}

int get_elevation(int origin, int x, int y, int smoothing)
{
	return smooth_avg(elevation_noise, origin, x, y, smoothing);
}

int temperature_noise(int origin, int x, int y)
{
	// Offset the origin
	origin += VIRTUAL_WIDTH/2;

	// Get high frequency noise
	int noise;
	noise = cbrng(origin + x + y * VIRTUAL_WIDTH) % 256;

	// Apply regional temperature modifiers
	if (y < CLIMATE_BAND_HEIGHT || y > HEIGHT - CLIMATE_BAND_HEIGHT)
		noise -= NORTH_POLE_TEMP_MOD;
	if (y > HEIGHT/2 - CLIMATE_BAND_HEIGHT && y < HEIGHT/2 + CLIMATE_BAND_HEIGHT)
		noise += EQUATOR_TEMP_MOD;

	// Apply low frequency noise
	noise += (cbrng((origin/LOD_FAC) + (x/LOD_FAC) + (y/LOD_FAC) * VIRTUAL_WIDTH) % LOD_IMPACT) - (LOD_IMPACT/2);

	return noise;
}

int get_temperature(int origin, int x, int y, int smoothing)
{
	return smooth_avg(temperature_noise, origin, x, y, smoothing);
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

enum terrain_type classify_terrain(int value)
{
#define SEA_LEVEL 128
#define OCEAN_BAND 30
#define SHALLOW_BAND 8
#define SAND_BAND 5
#define GRASS_BAND 10
#define WOODS_BAND 8
#define MOUNTAIN_BAND 12

#define BAND_CASE(type) \
	if (value < type##_BAND) \
		return type; \
	value -= type##_BAND;

	if (value < SEA_LEVEL) {
		value = SEA_LEVEL - value;
		// Water
		BAND_CASE(SHALLOW)
		BAND_CASE(OCEAN)
		return ABYSS;
	} else {
		value -= SEA_LEVEL;
		// Land
		BAND_CASE(SAND)
		BAND_CASE(GRASS)
		BAND_CASE(WOODS)
		BAND_CASE(MOUNTAIN)
		return PEAK;
	}
}

enum climate_type {
	COLD,
	TEMPERATE,
	HOT,
	NUM_CLIMATE_TYPES
};

enum terrain_type classify_climate(int value)
{
#define TEMPERATE_THRESHOLD 128
#define TEMPERATE_BAND 20

	if (value < TEMPERATE_THRESHOLD) {
		value = TEMPERATE_THRESHOLD - value;
		// Cold climate
		BAND_CASE(TEMPERATE)
		return COLD;
	} else {
		value -= TEMPERATE_THRESHOLD;
		// Hot climate
		BAND_CASE(TEMPERATE)
		return HOT;
	}
}


/* ******** TILE DISPLAY ******** */

char tile_char[NUM_TERRAIN_TYPES] = {
	[PEAK] = '^',
	[MOUNTAIN] = '=',
	[WOODS] = '%',
	[GRASS] = '"',
	[SAND] = '~',
	[SHALLOW] = '_',
	[OCEAN] = '_',
	[ABYSS] = ' '
};

const char *tile_color[NUM_CLIMATE_TYPES][NUM_TERRAIN_TYPES] = {
	[COLD] = {
		[PEAK] = FG_BCOLR(WHITE),
		[MOUNTAIN] = FG_BCOLR(BLACK),
		[WOODS] = FG_COLR(WHITE),
		[GRASS] = FG_BCOLR(WHITE),
		[SAND] = FG_BCOLR(WHITE),
		[SHALLOW] = FG_BCOLR(CYAN),
		[OCEAN] = FG_COLR(CYAN),
		[ABYSS] = FG_COLR(BLACK)
	},
	[TEMPERATE] = {
		[PEAK] = FG_BCOLR(WHITE),
		[MOUNTAIN] = FG_BCOLR(BLACK),
		[WOODS] = FG_COLR(GREEN),
		[GRASS] = FG_BCOLR(GREEN),
		[SAND] = FG_BCOLR(YELLOW),
		[SHALLOW] = FG_BCOLR(BLUE),
		[OCEAN] = FG_COLR(BLUE),
		[ABYSS] = FG_COLR(BLACK)
	},
	[HOT] = {
		[PEAK] = FG_BCOLR(BLACK),
		[MOUNTAIN] = FG_BCOLR(BLACK),
		[WOODS] = FG_COLR(YELLOW),
		[GRASS] = FG_BCOLR(YELLOW),
		[SAND] = FG_BCOLR(YELLOW),
		[SHALLOW] = FG_BCOLR(BLUE),
		[OCEAN] = FG_COLR(BLUE),
		[ABYSS] = FG_COLR(BLACK)
	}
};

void print_tile(int elevation, int temp)
{
	enum terrain_type tt = classify_terrain(elevation);
	enum climate_type ct = classify_climate(temp);
	printf(SGR("%s") "%c", tile_color[ct][tt], tile_char[tt]);
}


/* ******** WORLD DISPLAY ******** */

void print_world(int origin, int erosion, int offset)
{
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int elevation = get_elevation(origin, x, y, erosion);
			int temp = get_temperature(origin, x, y, erosion);
			print_tile(elevation + offset, temp);
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
	if (argc > 1)
		return 0;

	// Notice: no storage of depth map anywhere means area is effectively "infinite" barring virtual width
	// Could probably implement some interesting wrapping mechanism to simulate toroidal or other surfaces
	system("stty raw -echo isig");
	for (;;) {
		printf(ED("2") CUP("1","1"));
		print_world(origin, depth, offset);
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
			origin += 4 * VIRTUAL_WIDTH;
			break;
		case 'k':
		case '8':
			origin -= 4 * VIRTUAL_WIDTH;
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
