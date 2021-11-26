#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include "../aterm.h"

// TODO: Split this into multiple files

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

unsigned long long djb2(const char *str, size_t n)
{
	unsigned long long k = 5381;
	while (n --> 0)
		k = (k << 5) + k + *str++;
	return k;
}

enum src_mode {
	NONE = 0,
	ROTATE = 1 << 0,
	REFLECT = 1 << 1,
};

struct grid *rotate(struct grid *g)
{
	struct grid *r = create_grid(g->height, g->width);
	int max_y = g->height-1;
	for (int y = 0; y < g->height; y++)
		for (int x = 0; x < g->width; x++)
			XY(r, max_y-y, x) = XY(g, x, y);
	return r;
}

struct grid *hflip(struct grid *g)
{
	struct grid *r = create_grid(g->width, g->height);
	int max_x = g->width-1;
	for (int y = 0; y < g->height; y++)
		for (int x = 0; x < g->width; x++)
			XY(r, max_x-x, y) = XY(g, x, y);
	return r;
}

struct grid *vflip(struct grid *g)
{
	struct grid *r = create_grid(g->width, g->height);
	int max_y = g->height-1;
	for (int y = 0; y < g->height; y++)
		for (int x = 0; x < g->width; x++)
			XY(r, x, max_y-y) = XY(g, x, y);
	return r;
}

void add_if_unique(struct grid **arr, unsigned long long *hashes, int *count, struct grid *g)
{
	unsigned long long hash = djb2(g->space, g->width * g->height);
	int n = *count;
	int i;
	for (i = 0; i < n; i++)
		if (hash == hashes[i])
			break;
	// ^ TODO: Might want to record frequency of duplicate patterns
	if (i < n) {
		destroy_grid(g);
	} else {
		arr[n] = g;
		hashes[n] = hash;
		n++;
		*count = n;
	}
}

int patterns(struct grid ***arr_ptr, struct grid *src, int m, int n, enum src_mode mode)
{
	// TODO: Refactor this somehow
	// Extract all m by n slices from grid; place array in *arr_ptr
	int max_x = src->width - m + 1;
	int max_y = src->height - n + 1;
	int max_i = max_x * max_y * 7;
	struct grid **arr = malloc(sizeof(*arr_ptr) * max_i);
	unsigned long long *hashes = malloc(sizeof(*arr_ptr) * max_i);
	int count = 0;
	for (int y = 0; y < max_y; y++) {
		for (int x = 0; x < max_x; x++) {
			struct grid *g = slice(src, x, y, m, n);
			add_if_unique(arr, hashes, &count, g);
		}
	}
	if (mode & ROTATE) {
		// FIXME: This code assumes m == n
		for (int i = 0; i < count; i++) {
			struct grid *r90 = rotate(arr[i]);
			struct grid *r180 = rotate(r90);
			struct grid *r270 = rotate(r180);
			add_if_unique(arr, hashes, &count, r90);
			add_if_unique(arr, hashes, &count, r180);
			add_if_unique(arr, hashes, &count, r270);
		}
	}
	if (mode & REFLECT) {
		for (int i = 0; i < count; i++) {
			struct grid *h = hflip(arr[i]);
			struct grid *v = vflip(arr[i]);
			struct grid *t = hflip(v);
			add_if_unique(arr, hashes, &count, h);
			add_if_unique(arr, hashes, &count, v);
			add_if_unique(arr, hashes, &count, t);
		}
	}
	free(hashes);
	*arr_ptr = arr;
	return count;
}

struct wfc_gen *create_wfc_gen(struct grid *src, int m, int n, enum src_mode mode)
{
	struct wfc_gen *w = malloc(sizeof(*w));
	w->m = m;
	w->n = n;
	w->n_patterns = patterns(&w->patterns, src, m, n, mode);
	for (int i = 0; i < 256; i++)
		w->freq[i] = 0;
	for (int i = 0; i < src->width * src->height; i++)
		w->freq[(int)src->space[i]]++;
	// NB. Do not refactor this loop into being done any other way.
	// The ascending order must be preserved for entropy_equal() to work properly.
	w->n_states = 0;
	for (int c = 0; c < 256; c++) {
		if (w->freq[c] <= 0)
			continue;
		w->states[w->n_states++] = w->states[0];
		w->states[0] = (char)c;
	}
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
	bool final;
	int magnitude;
	bool queued;
	int n_states;
	char *states;
};

struct entropy *create_entropy(char *states, int n_states)
{
	struct entropy *e = malloc(sizeof(*e));
	e->final = false;
	e->magnitude = INT_MAX;
	e->queued = false;
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
	static bool init = false;
	if (!init) {
		printf(CLS);
		init = true;
	}
	printf(CUP("1","1"));
	for (int y = 0; y < w->height; y++) {
		for (int x = 0; x < w->width; x++) {
			struct entropy *e = XY(w, x, y);
			if (e->final)
				putchar(e->states[0]);
			else if (e->n_states < 1)
				printf(SGR(FG_COLR(RED)) "X" SGR(RESET));
			else
				printf(SGR(FG_COLR(BLUE)) "?" SGR(RESET));
		}
		putchar('\n');
	}
}

bool possible(struct entropy *e, char c)
{
	if (e->n_states < 1)
		return true; // Don't limit by contradictions
	if (e->final)
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

int locally_correct(struct wfc_gen *wfc, struct wave *w, int center_x, int center_y)
{
	// Returns local entropy, i.e. how many patterns fit
	// Calculate preferred loop indices
	int x0 = center_x - (wfc->m-1);
	int y0 = center_y - (wfc->n-1);
	int x1 = center_x;
	int y1 = center_y;
	// Perform range checks and correct OOB ranges
	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;
	if (x1 + wfc->m-1 >= w->width)
		x1 = w->width - wfc->m;
	if (y1 + wfc->n-1 >= w->height)
		y1 = w->height - wfc->n;
	// Check all patterns at every index
	int total_entropy = 0;
	for (int y = y0; y <= y1; y++) {
		for (int x = x0; x <= x1; x++) {
			int offset_entropy = 0;
			for (int i = 0; i < wfc->n_patterns; i++)
				if (agrees(w, x, y, wfc->patterns[i]))
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

void recalculate_entropy(struct wfc_gen *wfc, struct wave *w, int x, int y)
{
	struct entropy *e = XY(w, x, y);
	e->n_states = 0;
	e->magnitude = 0;
	int c = next_nonzero(wfc->freq, 256, 0);
	while (c < 256) {
		e->final = true;
		e->states[e->n_states++] = e->states[0];
		e->states[0] = (char)c;
		int partial_entropy = locally_correct(wfc, w, x, y);
		if (partial_entropy == 0)
			e->states[0] = e->states[--e->n_states];
		e->magnitude += partial_entropy;
		e->final = false;
		c = next_nonzero(wfc->freq, 256, c+1);
	}
	if (e->n_states == 1) // Capitalize on free observations
		e->final = true;
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

void append_neighbors(struct wfc_gen *wfc, struct wave *w, int x, int y, int *arr, int *n_arr)
{
	int n = *n_arr;
	for (int dy = -(wfc->n-1); dy <= (wfc->n-1); dy++) {
		for (int dx = -(wfc->m-1); dx <= (wfc->m-1); dx++) {
			if (x+dx < 0 || x+dx >= w->width)
				continue;
			if (y+dy < 0 || y+dy >= w->height)
				continue;
			if (dx == 0 && dy == 0)
				continue;
			int p = (x+dx) + (y+dy) * w->width;
			struct entropy *e = w->space[p];
			if (e->queued || e->final)
				continue;
			e->queued = true;
			arr[n++] = p;
		}
	}
	*n_arr = n;
}

void propagate(struct wfc_gen *wfc, struct wave *w, int x, int y)
{
	int *bag = malloc(sizeof(*bag) * w->width * w->height);
	int n_bag = 0;
	append_neighbors(wfc, w, x, y, bag, &n_bag);
	printf(CUD("1"));
	while (n_bag > 0) {
		printf(CUU("1"));
		printf(EL("2") CHA("1") "Propagations left: %d\n", n_bag);
		fflush(stdout);
		int rand_idx = rand() % n_bag;
		int pos = bag[rand_idx];
		bag[rand_idx] = bag[--n_bag];
		struct entropy *e = w->space[pos];
		struct entropy *tmp = create_entropy(wfc->states, wfc->n_states);
		w->space[pos] = tmp;
		int x = pos % w->width, y = pos / w->width;
		recalculate_entropy(wfc, w, x, y);
		if (tmp->magnitude > 1 && !entropy_equal(e, tmp))
			append_neighbors(wfc, w, x, y, bag, &n_bag);
		destroy_entropy(e);
	}
	free(bag);
}

void observe(struct wfc_gen *wfc, struct entropy *e)
{
	int i;
	int total_freq = 0;
	for (i = 0; i < e->n_states; i++)
		total_freq += wfc->freq[(int)e->states[i]];
	int chosen_freq = rand() % total_freq;
	for (i = 0; i < e->n_states; i++) {
		int f = wfc->freq[(int)e->states[i]];
		if (chosen_freq < f)
			break;
		else
			chosen_freq -= f;
	}
	e->states[0] = e->states[i];
	e->n_states = 1;
	e->final = true;
}

int find_min_entropy(struct wave *w)
{
	int min = INT_MAX;
	int num_mins = 0;
	int *mins = malloc(sizeof(*mins) * w->width * w->height);
	for (int i = 0; i < w->width * w->height; i++) {
		struct entropy *e = w->space[i];
		if (e->final)
			continue;
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

void unset_neighbors(struct wfc_gen *wfc, struct wave *w, int x, int y, int r)
{
	for (int dy = -r; dy <= r; dy++) {
		for (int dx = -r; dx <= r; dx++) {
			if (x+dx < 0 || x+dx >= w->width)
				continue;
			if (y+dy < 0 || y+dy >= w->height)
				continue;
			int pos = (x+dx) + (y+dy) * w->width;
			destroy_entropy(w->space[pos]);
			w->space[pos] = create_entropy(wfc->states, wfc->n_states);
		}
	}
	print_wave(w);
}

bool collapse(struct wfc_gen *wfc, struct wave *w)
{
	int contra = 0; // # of consecutive contradictions
	for (;;) {
		// TODO: See if it's worth returning x, y, and entropy from find_min_entropy
		print_wave(w);
		int pos = find_min_entropy(w);
		int x = pos % w->width, y = pos / w->width;
		if (pos < 0)
			break;
		if (w->space[pos]->n_states < 1) {
			unset_neighbors(wfc, w, x, y, contra);
			contra <<= 1;
		} else {
			observe(wfc, w->space[pos]);
			contra = 1;
		}
		propagate(wfc, w, x, y);
	}
	return true;
}

struct grid *proto_wfc(struct wfc_gen *wfc, int width, int height)
{
	struct wave *w = create_wave(width, height, wfc->states, wfc->n_states);
	if (!collapse(wfc, w)) {
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
	struct wfc_gen *w = create_wfc_gen(src, 2, 2, REFLECT);
	destroy_grid(src);
	// Generate map
	struct grid *map = proto_wfc(w, width, height);
	destroy_wfc_gen(w);
	// Print it
	if (map != NULL)
		destroy_grid(map);
	return 0;
}
