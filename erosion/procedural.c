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
	return x + (x >> 32);
}

#define VIRT_WIDTH 1000
#define WIDTH 50
#define HEIGHT 50

int get_depth(int origin, int x, int y, int erosion)
{
	// Here's the interesting idea: Procedurally generate in one step with recursion and counter-based RNG
	int sum = 0;
	if (erosion <= 0) {
		// Soft clamping
		if (x <= 0 || x >= WIDTH-1 || y <= 0 || y >= HEIGHT-1)
			return 0x40;
		return cbrng(origin + x + y * VIRT_WIDTH) % 256;
	}
	for (int dy = -1; dy <= 1; dy++)
		for (int dx = -1; dx <= 1; dx++)
			sum += get_depth(origin, x + dx, y + dy, erosion - 1);
	return sum / 9;
}

void shade(int height)
{
	if (height < 0x60) {
		printf(SGR(BG_COLR(BLACK)));
	} else if (height < 0x78) {
		printf(SGR(BG_COLR(BLUE)));
	} else if (height < 0x80) {
		printf(SGR(BG_BCOLR(BLUE)));
	} else if (height < 0x88) {
		printf(SGR(BG_BCOLR(YELLOW)));
	} else if (height < 0x90) {
		printf(SGR(BG_BCOLR(GREEN)));
	} else if (height < 0x98) {
		printf(SGR(BG_COLR(GREEN)));
	} else if (height < 0xa0) {
		printf(SGR(BG_BCOLR(BLACK)));
	} else {
		printf(SGR(BG_BCOLR(WHITE)));
	}
}

void print_depth(int origin, int erosion, int offset)
{
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int depth = get_depth(origin, x, y, erosion);
			shade(depth + offset);
			putchar(' ');
			putchar(' ');
		}
		printf(SGR(RESET) "\n\r");
	}
}

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
