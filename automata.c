//usr/bin/env tcc $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "aterm.h"

// TODO: Rewrite this to be faster

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
			if (g->field[i])
				printf(SGR(BG_BCOLR(WHITE)));
			else
				printf(SGR(BG_COLR(BLACK)));
			printf("  "SGR(RESET));
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

cell_t bosco(struct grid *g, size_t c)
{
	size_t w = g->width, h = g->height;
	size_t x0 = c % w, y0 = c / w;
	int alive = 0;
	for (int dy = -5; dy <= 5; dy++)
	for (int dx = -5; dx <= 5; dx++) {
		int x1 = (x0 + dx + w) % w;
		int y1 = (y0 + dy + h) % h;
		int i = x1 + y1 * w;
		if (i == c)
			continue;
		if (g->field[i])
			alive++;
	}
	if (g->field[c]) {
		// Survival
		return alive >= 33 && alive <= 57;
	} else {
		// Birth
		return alive >= 34 && alive <= 45;
	}
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

int get_dim(int x_factor, int y_factor)
{
	int x, y;
	system("stty raw");
	printf(CUP("999", "999") DSR);
	printf("\nPress enter to continue\n");
	printf("%d\n", scanf(CSI "%d" AND "%d" "R", &y, &x));
	system("stty sane");
	return (x-1) * x_factor + (y-1) * y_factor;
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	size_t w = get_dim(1, 0) / 2;
	size_t h = get_dim(0, 1);
	if (argc > 2) {
		w = atol(argv[1]);
		h = atol(argv[2]);
	}

	struct grid *g = new_grid(w, h);

	for (int i = 0; i < w * h; i++) {
		int r = (double)rand()/RAND_MAX < 0.5; // Fill factor
		g->field[i] = r;
	}

	printf(CLS);
	for (;;) {
		printf(CUP("1","1"));
		print_binary_grid(g);

		usleep(100000);
		step_grid(g, bosco);
	}

	destroy_grid(g);
	return 0;
}
