//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/random.h>


/* ******** COUNTER-BASED PRNG ******** */

uint64_t splitmix64_ctr(uint64_t key, uint64_t ctr)
{
	uint64_t z = (key + ctr * 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

/* ******** NOISE GENERATION ******** */

#define VIRTUAL_WIDTH 100000
#define WIDTH 80
#define HEIGHT 60

uint64_t noise_key = 0;
int noise(int origin, int x, int y, int smoothing)
{
	// Here's the interesting idea: Procedurally generate in one step with recursion and counter-based RNG
	if (smoothing <= 0) {
		return splitmix64_ctr(noise_key, origin + x + y * VIRTUAL_WIDTH) & 0xff;
	}
	// Calculate average recursively based on smoothing level
	int sum = 0;
	for (int dy = -1; dy <= 1; dy++)
		for (int dx = -1; dx <= 1; dx++)
			sum += noise(origin, x + dx, y + dy, smoothing - 1);
	return sum / 9;
}

void print_map(int origin, int erosion, int sealvl)
{
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int n = noise(origin, x, y, erosion);
			if (sealvl >= 0)
				n = (n > sealvl ? 255 : 0);
			printf("\033[48;2;%d;%d;%dm  ", n, n, n);
		}
		printf("\033[m\n\r");
	}
}


/* ******** MAIN / INPUT LOOP ******** */

void signal_handler(int signo)
{
	system("stty -raw echo");
	exit(0);
}

int main(int argc, char **argv)
{
	int origin = 0;
	char c;
	int depth = 3;
	if (argc > 1)
		return 0;

	if (0 > getrandom(&noise_key, sizeof(noise_key), 0))
		perror("getrandom()");

#ifndef INTERACTIVE
	print_map(origin, depth, -1);
	return 0;
#else
	signal(SIGINT, signal_handler);

	// Notice: no storage of depth map anywhere means area is effectively "infinite" barring virtual width
	// Could probably implement some interesting wrapping mechanism to simulate toroidal or other surfaces
	system("stty raw -echo isig");
	printf("\033[2J");
	int sealvl = -1;
	for (;;) {
		printf("\033[1;1H");
		print_map(origin, depth, sealvl);
		c = getchar();
		switch (c) {
		case '{':
			noise_key++;
			break;
		case '}':
			noise_key--;
			break;
		case '[':
			depth -= (depth > 0);
			break;
		case ']':
			depth += 1;
			break;
		case ' ':
			sealvl = (sealvl >= 0 ? -1 : 128);
			break;
		case '+':
			sealvl++;
			break;
		case '-':
			sealvl--;
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
#endif
}
