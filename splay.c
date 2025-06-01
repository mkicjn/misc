//$(which tcc) $CFLAGS -run $0 $@; exit $?
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

struct node {
	unsigned long key;
	void *val;
	struct node *child[2];
};

enum dir {
	// (used for child indexing)
	LEFT  = 0,
	RIGHT = 1,
	// (not used for child indexing)
	HERE,
	NOWHERE,
};

////////////////////////////////////////////////////////////////////////////////

void print_node(struct node *n, int depth, const char *s)
{
	if (n == NULL)
		return;

	for (int i = 1; i < depth; i++)
		printf("| ");
	printf("%s%lu\n", s, n->key);
	print_node(n->child[LEFT], depth+1, "L:");
	print_node(n->child[RIGHT], depth+1, "R:");
}

unsigned long comparisons = 0;
unsigned long rotations = 0;
void show_tree(struct node *root)
{
	(void)root;
#ifdef SHOW_TREES
	print_node(root, 0, "");
	printf("comparisons: %lu\n", comparisons);
	printf("rotations: %lu\n", rotations);
	comparisons = 0;
	rotations = 0;
	printf("----------------------------------------\n");
#endif
}

////////////////////////////////////////////////////////////////////////////////

/*
 * Move x where p was and swap links around
 * e.g., rotate(&g->child[LEFT], RIGHT);
 *
 *      (g)           (g)
 *      / \           / \
 *     p   d   ->    x   d
 *    / \           / \
 *   a   x         p   c
 *      / \       / \
 *     b   c     a   b
 *
 * All types of splays have this in common
 */
void rotate(struct node **p_ptr, int x_dir)
{
	rotations++;
	struct node *p = *p_ptr;
	struct node *x = p->child[x_dir];
	p->child[x_dir] = x->child[1-x_dir];
	x->child[1-x_dir] = p;
	*p_ptr = x;
}

// Local search
// Returns direction of X
static inline enum dir compare(struct node *n, unsigned long key)
{
	comparisons++;
	if (n == NULL)
		return NOWHERE;
	else if (key > n->key)
		return RIGHT;
	else if (key < n->key)
		return LEFT;
	else
		return HERE;
}

// Top-down strategy
bool splay(struct node **root_ptr, unsigned long key)
{
	if (*root_ptr == NULL)
		return false;

	struct node *subtree[2] = {NULL, NULL};
	struct node **leaf[2] = {&subtree[0], &subtree[1]};

	for (;;) {
		struct node *g = *root_ptr;
		enum dir dir = compare(g, key);
		if (dir == HERE)
			break;

		struct node *p = g->child[dir];
		if (compare(p, key) == dir) {
			g = p;
			rotate(root_ptr, dir);
		}

		if (g->child[dir] == NULL)
			break;
		*leaf[1-dir] = g;
		leaf[1-dir] = &g->child[dir];
		*root_ptr = g->child[dir];
	}

	struct node *g = *root_ptr;
	*leaf[LEFT] = g->child[LEFT];
	*leaf[RIGHT] = g->child[RIGHT];
	g->child[LEFT] = subtree[LEFT];
	g->child[RIGHT] = subtree[RIGHT];
	return g->key == key;
}

bool find(struct node **root_ptr, unsigned long key)
{
	if (!splay(root_ptr, key)) {
		printf("lookup failed\n");
		return false;
	} else {
		return true;
	}
}

bool insert(struct node **root_ptr, struct node *x)
{
	if (*root_ptr == NULL) {
		*root_ptr = x;
		return true;
	}

	if (splay(root_ptr, x->key))
		return false;

	struct node *g = *root_ptr;
	int dir = compare(g, x->key);
	x->child[1-dir] = g;
	x->child[dir] = g->child[dir];
	g->child[dir] = NULL;
	*root_ptr = x;
	return true;
}

bool delete(struct node **root_ptr, unsigned long key)
{
	if (!splay(root_ptr, key))
		return false;
	struct node *del = *root_ptr; // (To be removed)

	if (!del->child[RIGHT]) {
		*root_ptr = del->child[LEFT];
		return true;
	} else if (!del->child[LEFT]) {
		*root_ptr = del->child[RIGHT];
		return true;
	}
	*root_ptr = del->child[RIGHT];
	splay(root_ptr, 0); // 0 is the minimum unsigned value of any width
	(*root_ptr)->child[LEFT] = del->child[LEFT];
	return true;
}

// TODO: Customizable node data (maybe take inspiration from BSD headers?)
// TODO: Get/Set/Delete interface
// TODO: More rigorous testing / benchmarking

unsigned long levels = 0;
void show_path(struct node *n, unsigned long key)
{
	int dir = compare(n, key);
	switch (dir) {
	case NOWHERE:
		printf("?\n");
		break;
	case HERE:
		printf(".\n");
		break;
	default:
		levels++;
		printf("%c", "lr"[dir]);
		show_path(n->child[dir], key);
	}
}

int main(int argc, char **argv)
{
	// Base case testing
#define NODE(X, L, R) &(struct node){.key=(X), .child={(L), (R)}}
#define LEAF(X) NODE(X, NULL, NULL)
	struct node *root;

	root = 
		NODE(3,
			NODE(2,
				NODE(1,
					NULL,
					NULL),
				NULL),
			NULL);

	show_tree(root);
	splay(&root, 1);
	show_tree(root);

	root = 
		NODE(3,
			NODE(1,
				NULL,
				NODE(2,
					NULL,
					NULL)
			),
			NULL
		);

	show_tree(root);
	splay(&root, 2);
	show_tree(root);

	// Test splaying effect on unfavorably balanced trees
	
	// Oops, all zig-zig
	root =
		NODE(10,
			NODE(9,
				NODE(8,
					NODE(7,
						NODE(6,
							NODE(5,
								NODE(4,
									NODE(3,
										NODE(2,
											NODE(1,
												NULL,
												NULL),
											NULL),
										NULL),
									NULL),
								NULL),
							NULL),
						NULL),
					NULL),
				NULL),
			NULL);

	show_tree(root);
	splay(&root, 1);
	show_tree(root);

	// Oops, all zig-zag
	root =
		NODE(10,
			NODE(1,
				NULL, 
				NODE(9,
					NODE(2,
						NULL,
						NODE(8,
							NODE(3,
								NULL,
								NODE(7,
									NODE(4,
										NULL,
										NODE(6,
											NODE(5,
												NULL,
												NULL),
											NULL)),
									NULL)),
							NULL)),
					NULL)),
			NULL);

	show_tree(root);
	splay(&root, 5);
	show_tree(root);

	// Dynamic testing
	size_t num_trials = 1000;
	if (argc > 1)
		num_trials = atol(argv[1]);

#define INSERT(k) \
		do { \
			struct node *n = next_node++; \
			n->key = k; \
			n->child[0] = NULL; \
			n->child[1] = NULL; \
			insert(&root, n); \
			show_tree(root); \
		} while (0)

#define FIND(k) \
		do { \
			find(&root, k); \
			show_tree(root); \
		} while (0)

#define DELETE(k) \
		do { \
			delete(&root, k); \
			show_tree(root); \
		} while (0)

#define FREE_NODES() \
		do { \
			root = NULL; \
			next_node = pool; \
		} while (0)

#undef printf

	srand(time(NULL));
	clock_t dur = 0;

	// Allocate nodes (as many as requested)
	struct node *pool = malloc(num_trials * sizeof(struct node));
	struct node *next_node = pool;

	// Try out the sequential access theorem experimentally (try `grep comparisons`)
	dur = clock();
	for (size_t i = 0; i < num_trials; i++) {
		INSERT(i);
	}
	dur = (clock() - dur);
	printf("Average sequential insert duration: %fms\n", (dur / (double)CLOCKS_PER_SEC) / num_trials * 1000.0);

	dur = clock();
	for (size_t i = 0; i < num_trials; i++) {
		FIND(i);
	}
	dur = (clock() - dur);
	printf("Average sequential find duration: %fms\n", (dur / (double)CLOCKS_PER_SEC) / num_trials * 1000.0);

	// Try out randomized accesses
	dur = clock();
	for (size_t i = 0; i < num_trials; i++) {
		unsigned long n = rand() % (num_trials + 1);
		if (n >= num_trials)
			printf("(Expected failure) ");
		FIND(n);
	}
	dur = (clock() - dur);
	printf("Average randomized find duration: %fms\n", (dur / (double)CLOCKS_PER_SEC) / num_trials * 1000.0);

	dur = clock();
	for (size_t i = 0; i < num_trials; i++) {
		DELETE(i);
	}
	dur = (clock() - dur);
	printf("Average sequential delete duration: %fms\n", (dur / (double)CLOCKS_PER_SEC) / num_trials * 1000.0);

	// Reallocate nodes (exactly 10)
	free(pool);
	pool = malloc(10 * sizeof(*pool));
	FREE_NODES();

	// Manual testing, compare results online:
	// https://www.cs.usfca.edu/%7Egalles/visualization/SplayTree.html

	INSERT(1);
	INSERT(2);
	INSERT(3);
	INSERT(4);
	INSERT(5);
	INSERT(6);
	INSERT(7);
	FIND(4);
	FIND(3);
	FIND(2);
	FIND(3);
	FIND(6);
	FIND(1);
	DELETE(6);

	// Not found test
	FREE_NODES();
	printf("(Expected failure) ");
	FIND(~0ul);

	// Reallocate nodes (exactly 256)
	free(pool);
	pool = malloc(256 * sizeof(*pool));
	FREE_NODES();

	// Character tree traversal test
	if (argc > 2) {
		for (int i = 0; i < 255; i++)
			INSERT(i);
		unsigned long cs = 0;
		while (!feof(stdin)) {
			int c = getchar();
			if (c == EOF)
				break;
			show_path(root, c);
			splay(&root, c);
			cs++;
		}
		printf("characters: %lu\n", cs);
		printf("traversals: %lu\n", levels);
		printf("estimated encoding length: %f\n", (1.64 * levels) / 8.0);
	}

	free(pool);
	pool = NULL;
	FREE_NODES();

	return 0;
}
