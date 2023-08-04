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
		// Soft clamping
		if (x <= 0 || x >= WIDTH-1 || y <= 0 || y >= HEIGHT-1)
			return 0x40;
		*/
		return cbrng(origin + x + y * VIRT_WIDTH) % 256;
	}
	for (int dy = -1; dy <= 1; dy++)
		for (int dx = -1; dx <= 1; dx++)
			sum += get_depth(origin, x + dx, y + dy, erosion - 1);
	return sum / 9;
}

void shade(int height)
{
	if (height < 0x78) {
		printf(SGR(BG_COLR(BLUE)));
	} else if (height < 0x80) {
		printf(SGR(BG_BCOLR(BLUE)));
	} else if (height < 0x88) {
		printf(SGR(BG_BCOLR(YELLOW)));
	} else if (height < 0x98) {
		printf(SGR(BG_COLR(GREEN)));
	} else {
		printf(SGR(BG_BCOLR(BLACK)));
	}
}

void print_depth(int origin, int erosion)
{
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int depth = get_depth(origin, x, y, erosion);
			shade(depth);
			putchar(' ');
		}
		printf(SGR(RESET) "\n\r");
	}
}

int main(int argc, char **argv)
{
	int origin = cbrng(time(NULL));
	char c;
	int depth = 3;
	print_depth(origin, depth);
	if (argc > 1)
		return 0;

	// Notice: no storage of depth map anywhere means area is effectively "infinite" barring virtual width
	// Could probably implement some interesting wrapping mechanism to simulate toroidal or other surfaces
	system("stty raw -echo isig");
	for (;;) {
		printf(ED("2") CUP("1","1"));
		print_depth(origin, depth);
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
