#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>

struct grid {
	int width, height;
	char *space;
};

#define XY(g,x,y) ((g)->space[(x)+(y)*(g)->width])

struct grid *create_grid(int w, int h)
{
	struct grid *g = malloc(sizeof(*g));
	g->space = calloc(w * h, sizeof(*g->space));
	g->width = w;
	g->height = h;
	return g;
}

void destroy_grid(struct grid *g)
{
	free(g->space);
	free(g);
}

struct grid *file2grid(const char *filename)
{
	int area;
	char buf[200]; // TODO: Pick a real max
	FILE *f;
	struct grid *g = malloc(sizeof(*g));
	// Open file
	f = fopen(filename, "r");
	if (f == NULL)
		return NULL;
	// Read first line
	if (fgets(buf, sizeof(buf), f) == NULL)
		return NULL;
	// Detect width and height
	g->width = strlen(buf)-1; // -1 for '\n'
	fseek(f, 0, SEEK_END);
	g->height = ftell(f)/(g->width+1); // +1 for '\n'
	// Allocate space
	area = g->width * g->height;
	g->space = calloc(area+1, sizeof(char)); // +1 for last '\n'
	// Read file
	fseek(f, 0, SEEK_SET);
	for (int i = 0; i < area; i += g->width) { // each row
		int len;
		if (fgets(buf, sizeof(buf), f) == NULL) {
			destroy_grid(g);
			return NULL;
		}
		len = strlen(buf) - 1; // -1 for '\n'
		memcpy(&g->space[i], buf, len);
	}
	// Clean up and return
	fclose(f);
	return g;
}

void print_grid(struct grid *g)
{
	printf("~~~~~~~~~~~~~\n"); /////////////////////////
	for (int y = 0; y < g->height; y++) {
		for (int x = 0; x < g->width; x++) {
			char c = XY(g, x, y);
			if (c <= ' ')
				putchar(' ');
			else
				putchar(c);
		}
		putchar('\n');
	}
}

struct wfc_gen {
	int m, n; // Dimensions of patterns (is this necessary?)
	int num_patterns;
	int freq[256]; // Frequency of characters in the input image
	struct grid **patterns;
};

struct grid *slice(struct grid *g, int x0, int y0, int w, int h)
{
	// Extract one w by h slice from grid, top left corner at (x0, y0)
	// Allocate space
	struct grid *s = malloc(sizeof(*s));
	s->width = w;
	s->height = h;
	s->space = malloc(w * h * sizeof(*s->space));
	// Assign values
	int x1 = x0+w, y1 = y0+h;
	int i = 0;
	for (int y = y0; y < y1; y++)
		for (int x = x0; x < x1; x++)
			s->space[i++] = XY(g, x, y);
	return s;
}

int patterns(struct grid ***arr_ptr, struct grid *src, int m, int n)
{
	// Extract all m by n slices from grid; place array in *arr_ptr
	int max_x = src->width - m + 1;
	int max_y = src->height - n + 1;
	struct grid **arr = malloc(sizeof(*arr_ptr) * (max_x * max_y));
	int i = 0;
	for (int y = 0; y < max_y; y++)
		for (int x = 0; x < max_x; x++)
			arr[i++] = slice(src, x, y, m, n);
	*arr_ptr = arr;
	return i;
}

struct wfc_gen *create_wfc_gen(struct grid *src, int m, int n)
{
	struct wfc_gen *w = malloc(sizeof(*w));
	w->m = m;
	w->n = n;
	w->num_patterns = patterns(&w->patterns, src, m, n);
	for (int i = 0; i < 256; i++)
		w->freq[i] = 0;
	for (int i = 0; i < src->width * src->height; i++)
		w->freq[(int)src->space[i]]++;
	return w;
}

void destroy_wfc_gen(struct wfc_gen *w)
{
	for (int i = 0; i < w->num_patterns; i++)
		destroy_grid(w->patterns[i]);
	free(w->patterns);
	free(w);
}

struct entropy {
	bool collapsed : 1;
	int magnitude;
	char *states;
};

struct entropy *create_entropy(int max_entropy)
{
	struct entropy *e = malloc(sizeof(*e));
	e->collapsed = false;
	e->magnitude = 0;
	e->states = malloc(max_entropy);
	return e;
}

void destroy_entropy(struct entropy *e)
{
	free(e->states);
	free(e);
}

struct wave {
	int width, height;
	struct entropy **space;
};

struct wave *create_wave(int width, int height, int max_entropy)
{
	struct wave *w = malloc(sizeof(*w));
	w->width = width;
	w->height = height;
	w->space = malloc(sizeof(*w->space) * width * height);
	for (int i = 0; i < width * height; i++)
		w->space[i] = create_entropy(max_entropy);
	return w;
}

void destroy_wave(struct wave *w)
{
	for (int i = 0; i < w->width * w->height; i++)
		destroy_entropy(w->space[i]);
	free(w->space);
	free(w);
}

void print_wave(struct wave *w)
{
	for (int y = 0; y < w->height; y++) {
		for (int x = 0; x < w->width; x++) {
			struct entropy *e = XY(w, x, y);
			if (e->collapsed)
				putchar(e->states[0]);
			else
				printf("\033[31m?\033[m");
		}
		putchar('\n');
	}
}

bool possible(struct entropy *e, char c)
{
	return !e->collapsed || e->states[0] == c;
}

bool agrees(struct wave *w, int x, int y, struct grid *p)
{
	for (int dy = 0; dy < p->height; dy++)
		for (int dx = 0; dx < p->width; dx++)
			if (!possible(XY(w, x+dx, y+dy), XY(p, dx, dy)))
				return false;
	return true;
}

bool locally_correct(struct wfc_gen *gen, struct wave *w, int center_x, int center_y)
{
	// Calculate preferred loop indices
	int x0 = center_x - (gen->m-1);
	int y0 = center_y - (gen->n-1);
	int x1 = center_x;
	int y1 = center_y;
	// Perform range checks and correct OOB ranges
	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;
	if (x1 + gen->m-1 >= w->width)
		x1 = w->width - gen->m;
	if (y1 + gen->n-1 >= w->height)
		y1 = w->height - gen->n;
	// Check all patterns at every index
	for (int y = y0; y <= y1; y++) {
		for (int x = x0; x <= x1; x++) {
			bool good_pattern = false;
			for (int i = 0; i < gen->num_patterns; i++) {
				if (agrees(w, x, y, gen->patterns[i])) {
					good_pattern = true;
					break;
				}
			}
			if (!good_pattern)
				return false;
		}
	}
	return true;
}

int next_nonzero(int *arr, int max, int from)
{
	int i;
	for (i = from; i < max; i++)
		if (arr[i] != 0)
			break;
	return i;
}

void recalculate_entropy(struct wfc_gen *gen, struct wave *w, int x, int y)
{
	struct entropy *e = XY(w, x, y);
	e->magnitude = 0;
	int c = next_nonzero(gen->freq, 256, 0);
	while (c < 256) {
		e->collapsed = true;
		e->states[e->magnitude++] = e->states[0];
		e->states[0] = (char)c;
		if (!locally_correct(gen, w, x, y))
			e->states[0] = e->states[--e->magnitude];
		e->collapsed = false;
		c = next_nonzero(gen->freq, 256, c+1);
	}
}

void update_all_entropy(struct wfc_gen *gen, struct wave *w)
{
	for (int y = 0; y < w->height; y++)
		for (int x = 0; x < w->width; x++)
			if (!XY(w, x, y)->collapsed)
				recalculate_entropy(gen, w, x, y);
}

void observe(struct wave *w, int pos)
{
	struct entropy *e = w->space[pos];
	// TODO: Take frequency into account
	// Magnitude should never be 0 here; would be caught elsewhere
	e->states[0] = e->states[rand() % e->magnitude];
	e->magnitude = 1;
	e->collapsed = true;
}

int min_entropy(struct wave *w)
{
	int min = INT_MAX;
	int num_mins = 0;
	int *mins = malloc(sizeof(*mins) * w->width * w->height);
	for (int i = 0; i < w->width * w->height; i++) {
		struct entropy *e = w->space[i];
		if (e->collapsed)
			continue;
		if (e->magnitude < min) {
			min = e->magnitude;
			num_mins = 0;
		}
		if (e->magnitude == min)
			mins[num_mins++] = i;
	}
	if (num_mins < 1)
		return -1;
	min = mins[rand() % num_mins];
	free(mins);
	return min;
}

bool collapse(struct wfc_gen *gen, struct wave *w)
{
	for (;;) {
		update_all_entropy(gen, w);
		int min_pos = min_entropy(w);
		if (min_pos < 0)
			break;
		printf("Min entropy: %d @ %d\n", w->space[min_pos]->magnitude, min_pos);
		if (w->space[min_pos]->magnitude < 1) // Contradiction
			return false;
		observe(w, min_pos);
		print_wave(w);
	}
	return true;
}

struct grid *proto_wfc(struct wfc_gen *gen, int width, int height)
{
	struct wave *w = create_wave(width, height, gen->m * gen->n * gen->num_patterns);
	if (!collapse(gen, w)) {
		destroy_wave(w);
		return NULL;
	}
	struct grid *g = create_grid(width, height);
	for (int i = 0; i < width * height; i++)
		g->space[i] = w->space[i]->states[0];
	destroy_wave(w);
	return g;
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	int width, height;
	if (argc < 4)
		return 1;
	width = atoi(argv[2]);
	height = atoi(argv[3]);
	// Load input
	struct grid *src = file2grid(argv[1]);
	if (src == NULL)
		return 1;
	// Create model
	struct wfc_gen *w = create_wfc_gen(src, 3, 3);
	destroy_grid(src);
	// Generate map
	struct grid *map = proto_wfc(w, width, height);
	destroy_wfc_gen(w);
	// Print it
	if (map != NULL) {
		print_grid(map);
		destroy_grid(map);
	}
	return 0;
}
