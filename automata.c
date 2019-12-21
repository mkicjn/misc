#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned char cell_t;
struct grid {
	size_t width, height;
	cell_t *field;
};
typedef cell_t (*rule_t)(struct grid *, size_t);

struct grid *new_grid(size_t w, size_t h)
{
	struct grid *g = malloc(sizeof(struct grid));
	size_t a = w * h;
	cell_t *f = calloc(a, sizeof(cell_t));
	g->width = w;
	g->height = h;
	g->field = f;
	return g;
}

void destroy_grid(struct grid *g)
{
	free(g->field);
	free(g);
}

void step_grid(struct grid *g, rule_t r)
{
	size_t area = g->height * g->width;
	cell_t *tmp = malloc(area * sizeof(cell_t));
	for (int y = 0; y < g->height; y++)
	for (int x = 0; x < g->width; x++) {
		size_t i = x + y * g->width;
		tmp[i] = r(g, i);
	}
	free(g->field);
	g->field = tmp;
}

void print_binary_grid(struct grid *g)
{
	for (int y = 0; y < g->height; y++) {
		for (int x = 0; x < g->width; x++) {
			int i = x + y * g->width;
			putchar(g->field[i] ? '#' : ' ');
		}
		putchar('\n');
	}
}

cell_t conway(struct grid *g, size_t c)
{ // Conway's Game of Life on a toroidal field
	size_t w = g->width, h = g->height;
	int alive = 0;
	for (int dy = -1; dy <= 1; dy++)
	for (int dx = -1; dx <= 1; dx++) {
		int x = (c % w + dx + w) % w;
		int y = (c / w + dy + h) % h;
		int i = x + y * w;
		if (i == c)
			continue;
		if (g->field[x + y * w])
			alive++;
	}
	if (alive == 3)
		return 1;
	else if (alive == 2)
		return g->field[c];
	else
		return 0;
}

void tsleep(clock_t n)
{
	clock_t s = clock();
	for (;;)
		if (clock() - s > n)
			break;
}

#include "aterm.h"

int main(int argc, char **argv)
{
	srand(time(NULL));
	const size_t w = 80, h = 24;
	struct grid *g = new_grid(w, h);

	for (int i = 0; i < w * h; i++) {
		int r = rand() % 2;
		printf("Initializing %d to %d\n", i, r);
		g->field[i] = r;
	}

	for (int i = 0; i < 100; i++) {
		printf(CSI CUH CSI CLS CSI AT_XY(1,1));
		print_binary_grid(g);
		step_grid(g, conway);
		putchar('\n');
		tsleep(200000);
		printf(CSI CUS);
	}

	destroy_grid(g);
	return 0;
}
