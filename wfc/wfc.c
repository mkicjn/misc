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
	int freq[256]; // Frequency of characters in the input image
	int n_states;
	char states[256];
	int n_patterns;
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
	w->n_states = 0;
	w->n_patterns = patterns(&w->patterns, src, m, n);
	for (int i = 0; i < 256; i++)
		w->freq[i] = 0;
	for (int i = 0; i < src->width * src->height; i++)
		w->freq[(int)src->space[i]]++;
	// NB. Do not refactor this loop into being done any other way.
	// The ascending order must be preserved for entropy_equal() to work properly.
	for (int c = 0; c < 256; c++)
		if (w->freq[c] > 0)
			w->states[w->n_states++] = (char)c;
	return w;
}

void destroy_wfc_gen(struct wfc_gen *w)
{
	for (int i = 0; i < w->n_patterns; i++)
		destroy_grid(w->patterns[i]);
	free(w->patterns);
	free(w);
}

struct entropy {
	bool collapsed;
	int magnitude;
	bool updated;
	int n_states;
	char *states;
};

struct entropy *create_entropy(char *states, int n_states)
{
	struct entropy *e = malloc(sizeof(*e));
	e->collapsed = false;
	e->magnitude = -1; // uncalculated
	e->updated = false;
	e->n_states = n_states;
	e->states = malloc(n_states);
	memcpy(e->states, states, n_states);
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

struct wave *create_wave(int width, int height, char *states, int n_states)
{
	struct wave *w = malloc(sizeof(*w));
	w->width = width;
	w->height = height;
	w->space = malloc(sizeof(*w->space) * width * height);
	for (int i = 0; i < width * height; i++)
		w->space[i] = create_entropy(states, n_states);
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
			else if (e->n_states < 1)
				printf("\033[31mX\033[m");
			else
				printf("\033[34m?\033[m");
		}
		putchar('\n');
	}
}

bool possible(struct entropy *e, char c)
{
	if (e->n_states < 0) // i.e. uncalculated
		return true;
	if (e->collapsed)
		return c == e->states[0];
	for (int i = 0; i < e->n_states; i++)
		if (c == e->states[i])
			return true;
	return false;
}

bool agrees(struct wave *w, int x, int y, struct grid *p)
{
	for (int dy = 0; dy < p->height; dy++)
		for (int dx = 0; dx < p->width; dx++)
			if (!possible(XY(w, x+dx, y+dy), XY(p, dx, dy)))
				return false;
	return true;
}

int locally_correct(struct wfc_gen *gen, struct wave *w, int center_x, int center_y)
{
	// Returns local entropy, i.e. how many patterns fit
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
	int total_entropy = 0;
	for (int y = y0; y <= y1; y++) {
		for (int x = x0; x <= x1; x++) {
			int offset_entropy = 0;
			for (int i = 0; i < gen->n_patterns; i++)
				if (agrees(w, x, y, gen->patterns[i]))
					offset_entropy++;
			if (offset_entropy == 0)
				return 0;
			total_entropy += offset_entropy;
		}
	}
	return total_entropy;
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
	e->n_states = 0;
	e->magnitude = 0;
	int c = next_nonzero(gen->freq, 256, 0);
	while (c < 256) {
		e->collapsed = true;
		e->states[e->n_states++] = e->states[0];
		e->states[0] = (char)c;
		int partial_entropy = locally_correct(gen, w, x, y);
		if (partial_entropy == 0)
			e->states[0] = e->states[--e->n_states];
		e->magnitude += partial_entropy;
		e->collapsed = false;
		c = next_nonzero(gen->freq, 256, c+1);
	}
	if (e->n_states == 1) // Capitalize on free observations
		e->collapsed = true;
}

bool entropy_equal(struct entropy *a, struct entropy *b)
{
	if (a->n_states != b->n_states)
		return false;
	// NB. This only works if the order of states is the same
	// The order will be the same if calculated through recalculate_entropy.
	// TODO: Decide if a more sophisticated approach is in order.
	for (int i = 0; i < a->n_states; i++)
		if (a->states[i] != b->states[i])
			return false;
	return true;
}

void push_neighbors(struct wfc_gen *gen, struct wave *w, int x, int y, int *stack, int *n_stack)
{
	int n = *n_stack;
	for (int dy = -(gen->n-1); dy <= (gen->n-1); dy++) {
		for (int dx = -(gen->m-1); dx <= (gen->m-1); dx++) {
			if (x+dx < 0 || x+dx >= w->width)
				continue;
			if (y+dy < 0 || y+dy >= w->height)
				continue;
			if (dx == 0 && dy == 0)
				continue;
			int p = (x+dx) + (y+dy) * w->width;
			struct entropy *e = w->space[p];
			if (e->collapsed || e->updated)
				continue;
			e->updated = true;
			stack[n++] = p;
		}
	}
	*n_stack = n;
}

void propagate(struct wfc_gen *gen, struct wave *w, int x, int y)
{
	int *stack = malloc(sizeof(*stack) * w->width * w->height);
	int n_stack = 0;
	push_neighbors(gen, w, x, y, stack, &n_stack);
	while (n_stack > 0) {
		int pos = stack[--n_stack];
		struct entropy *e = w->space[pos];
		struct entropy *tmp = create_entropy(gen->states, gen->n_states);
		w->space[pos] = tmp;
		int x = pos % w->width, y = pos / w->width;
		recalculate_entropy(gen, w, x, y);
		/**/
		if (tmp->magnitude < 1) {
			printf("Contradiction detected at %d, %d\n", x, y);
			return;
		}
		/**/
		if (!entropy_equal(e, tmp))
			push_neighbors(gen, w, x, y, stack, &n_stack);
		destroy_entropy(e);
	}
	free(stack);
}

void observe(struct wave *w, int pos)
{
	struct entropy *e = w->space[pos];
	// TODO: Take frequency into account
	if (e->n_states > 1) {
		// Magnitude should never be 0 here; would be caught elsewhere
		e->states[0] = e->states[rand() % e->n_states];
		e->n_states = 1;
	}
	e->collapsed = true;
}

int find_min_entropy(struct wave *w)
{
	int min = INT_MAX;
	int num_mins = 0;
	int *mins = malloc(sizeof(*mins) * w->width * w->height);
	for (int i = 0; i < w->width * w->height; i++) {
		struct entropy *e = w->space[i];
		if (e->collapsed)
			continue;
		e->updated = false;
		if (e->magnitude < min) {
			min = e->magnitude;
			num_mins = 0;
		}
		if (e->magnitude == min)
			mins[num_mins++] = i;
	}
	if (num_mins < 1) {
		free(mins);
		return -1;
	}
	min = mins[rand() % num_mins];
	free(mins);
	return min;
}

bool collapse(struct wfc_gen *gen, struct wave *w)
{
	for (;;) {
		print_wave(w);
		// TODO: See if it's worth returning x, y, and entropy from find_min_entropy
		int min_pos = find_min_entropy(w);
		if (min_pos < 0)
			break;
		printf("Min entropy: %d @ %d\n", w->space[min_pos]->n_states, min_pos);
		if (w->space[min_pos]->n_states < 1) // Contradiction
			return false;
		observe(w, min_pos);
		// TODO: The first call to propagate seems to take a long time. Investigate.
		propagate(gen, w, min_pos % w->width, min_pos / w->width);
	}
	return true;
}

struct grid *proto_wfc(struct wfc_gen *gen, int width, int height)
{
	struct wave *w = create_wave(width, height, gen->states, gen->n_states);
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
