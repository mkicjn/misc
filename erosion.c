#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

void grid_erode(struct grid *g)
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
			*grid_at(tmp, x0, y0) = (sum > num/2);
		}
	for (int y = 0; y < g->height; y++)
		for (int x = 0; x < g->width; x++)
			*grid_at(g, x, y) = *grid_at(tmp, x, y);
	grid_destroy(tmp);
}


void land_print(struct grid *g)
{
	printf(ED("2") CUP("1","1"));
	for (int y = 0; y < g->height; y++) {
		for (int x = 0; x < g->width; x++) {
			int val = *grid_at(g, x, y);
			switch (val) {
			// TODO Any better colors for terrain?
			case 0:
				printf(SGR(BG_BCOLR(BLUE)));
				break;
			case 1:
				printf(SGR(BG_BCOLR(YELLOW)));
				break;
			case 2:
				printf(SGR(BG_BCOLR(GREEN)));
				break;
			case 3:
				printf(SGR(BG_COLR(GREEN)));
				break;
			default:
				if (val < 0)
					printf(SGR(BG_COLR(BLUE)));
				else
					printf(SGR(BG_BCOLR(BLACK)));
				break;
			}
			printf("  ");
			printf(SGR(RESET));
		}
		putchar('\n');
	}
}

struct grid *land_new(double fill_rate, int width, int height)
{
	struct grid *g = grid_new(width, height);
	grid_randomize(g, fill_rate);
	for (int i = 0; i < 3; i++) {
		grid_erode(g);
		//land_print(g);
		//usleep(500000);
	}
	return g;
}

void land_compose(struct grid *dst, struct grid *src)
{
	int w = MIN(dst->width, src->width);
	int h = MIN(dst->height, src->height);
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
			if (*grid_at(dst, x, y) > 0)
				*grid_at(dst, x, y) += *grid_at(src, x, y);
			else
				*grid_at(dst, x, y) -= *grid_at(src, x, y);
}


#define COUNT(X) (sizeof(X)/sizeof(*(X)))

int main()
{
	srand(time(NULL));

	struct grid *lands[8]; // TODO Change number to > 1 to activate remaining logic
	lands[0] = land_new(0.5, 64, 64);
	//land_compose(lands[0], lands[0]);
	for (int i = 1; i < COUNT(lands); i++)
		lands[i] = land_new(0.5 / ((i+1)/2), 64, 64); // TODO: Find good heuristic for first argument

	for (int i = 1; i < COUNT(lands); i++)
		land_compose(lands[0], lands[i]);
	
	land_print(lands[0]);

	return 0;
}
