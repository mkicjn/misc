#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include "../aterm.h"

uint32_t cbrng(uint64_t n)
{ // Custom counter-based PRNG
	static uint64_t const s = 0xba2c2cab;

	uint64_t x = n * s;
	x *= x ^ s;
	return x >> 32;
}

#define VIRT_WIDTH 1000
#define WIDTH 80
#define HEIGHT 24

int get_depth(int origin, int x, int y, int erosion)
{
	// Here's the interesting idea: Procedurally generate in one step with recursion and counter-based RNG
	int sum = 0;
	if (erosion == 0) {
		/*
		// Clamping
		if (x <= 0 || x >= WIDTH-1 || y <= 0 || y >= HEIGHT-1)
			return 0;
		*/
		return cbrng(origin + x + y * VIRT_WIDTH) % 256;
	}
	for (int dy = -1; dy <= 1; dy++)
		for (int dx = -1; dx <= 1; dx++)
			sum += get_depth(origin, x + dx, y + dy, erosion - 1);
	return sum / 9;
}

void print_depth(int origin, int erosion)
{
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int shade = get_depth(origin, x, y, erosion);
			shade = (shade > 128 ? 255 : 0); // Comment out to print raw depth map
			printf(SGR(BG_COLR(CUSTOM COLR_RGB("%d","%d","%d"))) " ", shade, shade, shade);
		}
		printf(SGR(RESET) "\n\r");
	}
}

int main(int argc, char **argv)
{
	int origin = cbrng(time(NULL));
	char c;
	print_depth(origin, 2);
	if (argc > 1)
		return 0;

	// Notice: no storage of depth map anywhere means area is effectively "infinite" barring virtual width
	// Could probably implement some interesting wrapping mechanism to simulate toroidal or other surfaces
	system("stty raw -echo isig");
	for (;;) {
		printf(ED("2") CUP("1","1"));
		print_depth(origin, 2);
		c = getchar();
		switch (c) {
		case 'h':
		case '4':
			origin--;
			break;
		case 'j':
		case '2':
			origin += VIRT_WIDTH;
			break;
		case 'k':
		case '8':
			origin -= VIRT_WIDTH;
			break;
		case 'l':
		case '6':
			origin++;
			break;
		case 'q':
			system("stty -raw echo");
			return 0;
		}
	}
}
