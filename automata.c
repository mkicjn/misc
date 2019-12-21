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
			printf(g->field[i] ? "[]" : "  ");
		}
		putchar('\n');
	}
}

#include <unistd.h>
#include "aterm.h"

cell_t conway(struct grid *g, size_t c)
{ // Conway's Game of Life on a toroidal field
	size_t w = g->width, h = g->height;
	size_t x0 = c % w, y0 = c / w;
	int alive = 0;
	for (int dy = -1; dy <= 1; dy++)
	for (int dx = -1; dx <= 1; dx++) {
		int x1 = (x0 + dx + w) % w;
		int y1 = (y0 + dy + h) % h;
		int i = x1 + y1 * w;
		if (i == c)
			continue;
		if (g->field[i])
			alive++;
	}
	if (alive == 3)
		return 1;
	else if (alive == 2)
		return g->field[c];
	else
		return 0;
}

cell_t erosion(struct grid *g, size_t c)
{
	size_t w = g->width, h = g->height;
	size_t x0 = c % w, y0 = c / w;
	int alive = 0;
	for (int dy = -1; dy <= 1; dy++)
	for (int dx = -1; dx <= 1; dx++) {
		int x1 = (x0 + dx + w) % w;
		int y1 = (y0 + dy + h) % h;
		int i = x1 + y1 * w;
		if (g->field[i])
			alive++;
	}
	return alive > 5;
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	size_t w = 50, h = 50;
	if (argc > 2) {
		w = atol(argv[1]);
		h = atol(argv[2]);
	}

	struct grid *g = new_grid(w, h);

	for (int i = 0; i < w * h; i++) {
		int r = (rand() % 5) > 1;
		g->field[i] = r;
	}

	printf(CSI CLS);
	for (;;) {
		printf(AT_XY(1,1));
		print_binary_grid(g);

		usleep(1000000);
		step_grid(g, erosion);
	}

	destroy_grid(g);
	return 0;
}
