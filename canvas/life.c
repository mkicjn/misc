/*
 *	Adapted from ../automata.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "src/canvas.h"

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
	#pragma omp parallel for
	for (size_t y = 0; y < g->height; y++)
	for (size_t x = 0; x < g->width; x++) {
		size_t i = x + y * g->width;
		tmp[i] = r(g, i);
	}
	free(g->field);
	g->field = tmp;
}

void display_binary_grid(struct grid *g)
{
	#pragma omp parallel for
	for (size_t y = 0; y < g->height; y++) {
		for (size_t x = 0; x < g->width; x++) {
			size_t i = x + y * g->width;
			PX(x, y) = g->field[i] ? ~0 : 0;
		}
	}
	video_update();
}

cell_t conway(struct grid *g, size_t c)
{ // Conway's Game of Life on a toroidal field
	size_t w = g->width, h = g->height;
	size_t x0 = c % w, y0 = c / w;
	int alive = 0;
	for (int dy = -1; dy <= 1; dy++)
	for (int dx = -1; dx <= 1; dx++) {
		size_t x1 = (x0 + dx + w) % w;
		size_t y1 = (y0 + dy + h) % h;
		size_t i = x1 + y1 * w;
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

int main(int argc, char **argv)
{
	double period = 1.0 / 30.0;
	srand(time(NULL));
	if (argc > 1)
		period = 1.0 / atof(argv[1]);

	struct grid *g = new_grid(CANVAS_WIDTH, CANVAS_HEIGHT);

	for (size_t i = 0; i < CANVAS_AREA; i++) {
		int r = rand() & 1;
		g->field[i] = r;
	}

	video_start();
	display_binary_grid(g);
	tick();
	while (!user_quit()) {
		if (tock() < period)
			continue;
		tick();

		step_grid(g, conway);
		display_binary_grid(g);
		//printf("Step and render took %f ms\n", 1000.0 * tock());
	}

	destroy_grid(g);
	video_stop();
	return 0;
}
