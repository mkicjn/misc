//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../aterm.h"

#define WIDTH 80
#define HEIGHT 24

void grid_init(int grid[WIDTH][HEIGHT])
{
	for (int y = 0; y < HEIGHT; y++)
		for (int x = 0; x < WIDTH; x++)
			grid[x][y] = rand() % 256;

	for (int x = 0; x < WIDTH; x++) {
		grid[x][0] = 0;
		grid[x][HEIGHT-1] = 0;
	}
	for (int y = 0; y < HEIGHT; y++) {
		grid[0][y] = 0;
		grid[WIDTH-1][y] = 0;
	}
}

void grid_print(int grid[WIDTH][HEIGHT], int threshold)
{
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			int shade = grid[x][y];
			if (threshold >= 0)
				shade = (shade > threshold ? 255 : 0);
			printf(SGR(BG_COLR(CUSTOM COLR_RGB("%d","%d","%d"))) " ", shade, shade, shade);
		}
		printf(SGR(RESET) "\n");
	}
}

int grid_erode_point(int grid[WIDTH][HEIGHT], int x, int y)
{
	int sum = 0;
	for (int dx = -1; dx <= 1; dx++)
		for (int dy = -1; dy <= 1; dy++)
			sum += grid[x + dx][y + dy];
	return sum / 9;
}

void grid_erode(int grid[WIDTH][HEIGHT])
{
	int grid2[WIDTH][HEIGHT];
	for (int y = 1; y < HEIGHT-1; y++)
		for (int x = 1; x < WIDTH-1; x++)
			grid2[x][y] = grid_erode_point(grid, x, y);
	for (int y = 1; y < HEIGHT-1; y++)
		for (int x = 1; x < WIDTH-1; x++)
			grid[x][y] = grid2[x][y];
}

int grid_threshold_popcnt(int grid[WIDTH][HEIGHT], int threshold)
{
	int pop = 0;
	for (int y = 0; y < HEIGHT; y++)
		for (int x = 0; x < WIDTH; x++)
			pop += (grid[x][y] > threshold);
	return pop;
}

int grid_smart_threshold(int grid[WIDTH][HEIGHT], float frac)
{
	// Here's the intereting idea: Auto-balance sea level (and others)
	int lo = 0, hi = 256;
	while (hi - lo > 1) {
		int mid = (hi + lo) / 2;
		if (grid_threshold_popcnt(grid, mid) > (HEIGHT * WIDTH) * frac)
			lo = mid;
		else
			hi = mid;
	}
	return lo;
}

int main()
{
	int grid[WIDTH][HEIGHT];
	srand(time(NULL));

	grid_init(grid);
	for (int i = 0; i < 2; i++) {
		grid_print(grid, 128);
		grid_erode(grid);
		usleep(500e3);
	}
	grid_print(grid, 128);

	// TODO: Add variadic function for multiple thresholds and colors
	int land_threshold = grid_smart_threshold(grid, 0.3);
	printf("Rebalanced to %d\n", land_threshold);
	grid_print(grid, land_threshold);
	grid_print(grid, -1);

	return 0;
}
