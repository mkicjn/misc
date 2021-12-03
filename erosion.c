#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "aterm.h"

struct grid {
	int width, height;
	int spots[];
};

struct grid *grid_new(int width, int height)
{
	struct grid *g = malloc(sizeof(*g) + width * height * sizeof(*g->spots));
	g->width = width;
	g->height = height;
	memset(g->spots, 0, width * height * sizeof(*g->spots));
	return g;
}

void grid_destroy(struct grid *g)
{
	free(g);
}

int *grid_at(struct grid *g, int x, int y)
{
	return &g->spots[x + y * g->width];
}

void grid_randomize(struct grid *g, double fill_factor)
{
	for (int y = 0; y < g->height; y++)
		for (int x = 0; x < g->width; x++)
			*grid_at(g, x, y) = ((double)rand() / RAND_MAX < fill_factor);
}

#define MIN(X,Y) ((X)<(Y)?(X):(Y))
#define MAX(X,Y) ((X)>(Y)?(X):(Y))

void grid_erode(struct grid *g, bool binary)
{
	struct grid *tmp = grid_new(g->width, g->height);
	for (int y0 = 0; y0 < g->height; y0++)
		for (int x0 = 0; x0 < g->width; x0++) {
			int num = 0;
			int sum = 0;
			for (int y1 = MAX(0, y0-1); y1 <= MIN(g->width-1, y0+1); y1++)
				for (int x1 = MAX(0, x0-1); x1 <= MIN(g->width-1, x0+1); x1++) {
					num++;
					sum += *grid_at(g, x1, y1);
				}
			if (binary)
				*grid_at(tmp, x0, y0) = sum > num/2;
			else
				*grid_at(tmp, x0, y0) = sum / (num/2);
		}
	for (int y = 0; y < g->height; y++)
		for (int x = 0; x < g->width; x++)
			*grid_at(g, x, y) = *grid_at(tmp, x, y);
	grid_destroy(tmp);
}

void grid_plot(struct grid *g, int min, int max)
{
	double factor = 256.0 / (double)(max - min);
	printf(ED("2") CUP("1","1"));
	for (int y = 0; y < g->height; y++) {
		for (int x = 0; x < g->width; x++) {
			int shade = (int)(factor * (double)(*grid_at(g, x, y) - min));
			printf(SGR(BG_COLR(CUSTOM COLR_RGB("%d","%d","%d"))), shade, shade, shade);
			printf("  " SGR(RESET));
		}
		putchar('\n');
	}
}

#define COUNT(X) (sizeof(X)/sizeof(*(X)))

int main()
{
	srand(time(NULL));

	struct grid *land_a = grid_new(64, 64);

	grid_randomize(land_a, 0.5);
	grid_plot(land_a, 0, 1);
	usleep(1000000);

	for (int i = 1; i <= 2; i++) {
		grid_erode(land_a, true);
		grid_plot(land_a, 0, 1);
		usleep(1000000);
	}

	for (int i = 1; i <= 2; i++) {
		grid_erode(land_a, false);
		grid_plot(land_a, 0, 2*i);
		usleep(1000000);
	}

	return 0;
}
