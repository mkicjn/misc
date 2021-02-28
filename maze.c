#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

struct maze {
	int width, height;
	char tile[];
};

struct maze *new_maze(int w, int h)
{
	struct maze *m = malloc(sizeof(*m) + w*h*sizeof(m->tile[0]));
	m->width = w;
	m->height = h;
	return m;
}
#define free_maze(m) free(m)

void maze_initialize(struct maze *m)
{	// Fill maze with walls, i.e. '#'
	int area = m->width * m->height;
	for (int i = 0; i < area; i++)
		m->tile[i] = '#';
}

void maze_display(struct maze *m)
{
	/*
	for (int i = 0; i < m->height; i++)
		printf("%.*s\n", m->width, &m->tile[i*m->width]);
	*/
	for (int y = 0; y < m->height; y++) {
		for (int x = 0; x < m->width; x++) {
			int p = x + y * m->width;
			if (m->tile[p] == '#')
				printf("\033[47m  ");
			else
				printf("\033[40m  ");
		}
		printf("\033[m\n");
	}
}

#define NORTH(m,p) ((p) - 2*(m)->width)
#define SOUTH(m,p) ((p) + 2*(m)->width)
#define WEST(m,p) ((p) - 2)
#define EAST(m,p) ((p) + 2)

#define X(m,p) ((p)%(m)->width)
#define Y(m,p) ((p)/(m)->width)
#define MINX(m) (1)
#define MINY(m) (1)
#define MAXX(m) ((m)->width - 1)
#define MAXY(m) ((m)->height - 1)

int rand_neighbor(struct maze *m, int pos, char c)
{
	// Find all neighbors of pos equal to c
	int n = 0;
	int neighbors[4];
	if (Y(m,pos)-2 >= MINY(m) && m->tile[NORTH(m,pos)] == c)
		neighbors[n++] = NORTH(m,pos);
	if (Y(m,pos)+2 <  MAXY(m) && m->tile[SOUTH(m,pos)] == c)
		neighbors[n++] = SOUTH(m,pos);
	if (X(m,pos)+2 <  MAXX(m) && m->tile[EAST(m,pos)] == c)
		neighbors[n++] = EAST(m,pos);
	if (X(m,pos)-2 >= MINX(m) && m->tile[WEST(m,pos)] == c)
		neighbors[n++] = WEST(m,pos);
	// Return the position of a random neighbor
	// Return -1 if no such neighbor exists
	if (n == 0)
		return -1;
	else 
		return neighbors[rand()%n];
}

bool snake_continue(struct maze *m, int *pos)
{
	// Find all visited tiles adjacent to an unvisited tile
	int n = 0;
	int *candidate = malloc(sizeof(*candidate) * (m->width/2)*(m->height/2));
	for (int y = 1; y < MAXY(m); y += 2) {
		for (int x = 1; x < MAXX(m); x += 2) {
			int pos = x + y * m->width;
			int neighbor;
			if (!(m->tile[pos] == ' '))
				continue;
			neighbor = rand_neighbor(m, pos, '#');
			if (neighbor > 0)
				candidate[n++] = pos;
		}
	}
	// Return false if there are no such tiles
	if (n <= 0)
		return false;
	// Update *pos with the position of a random such tile
	*pos = candidate[rand() % n];
	m->tile[*pos] = ' ';
	return true;
}

static inline int wall(int p1, int p2)
{
	return p1 + (p2 - p1)/2;
}

bool snake_step(struct maze *m, int *pos)
{
	// Pick a neighbor who hasn't been visited yet
	int new_pos = rand_neighbor(m, *pos, '#');
	// Return false if there are no possible moves
	if (new_pos < 0)
		return false;
	// Step to the new position
	m->tile[new_pos] = ' ';
	m->tile[wall(*pos, new_pos)] = ' ';
	// Update position
	*pos = new_pos;
	return true;
}

int random_point(struct maze *m)
{
	int x = rand() % (m->width/2 - 1);
	int y = rand() % (m->height/2 - 1);
	x = 2 * x + 1;
	y = 2 * y + 1;
	return x + y * m->width;
}

void maze_generate(struct maze *m)
{
	// Variation on the "Hunt and Kill" algorithm.
	// I call it the "Snake" algorithm.
	// No vertical scanning; instead pick any unvisited square adjacent to visited.

	// Create the first blank space
	//int pos = m->width + 1;
	int pos = random_point(m);
	m->tile[pos] = ' ';
	// Until there are no free spaces
	do {
		// Walk from the space as long as possible
		while (snake_step(m, &pos))
			continue;
		// Jump to another valid point when necessary
	} while (snake_continue(m, &pos));
}

int main(int argc, char **argv)
{
	struct maze *m;
	int w = 80, h = 24;
	srand(time(NULL));
	if (argc > 2) {
		w = atoi(argv[1]);
		h = atoi(argv[2]);
	}
	m = new_maze(w/2, h);
	maze_initialize(m);
	maze_generate(m);
	maze_display(m);
	free_maze(m);
	return 0;
}
