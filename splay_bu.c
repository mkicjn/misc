//$(which tcc) $CFLAGS -run $0 $@; exit $?

// (Old bottom-up implementation)

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
unsigned long rotations = 0;
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
unsigned long comparisons = 0;
static inline enum dir compare(struct node *n, unsigned long key)
{
	comparisons++;
	if (n == NULL)
		return NOWHERE;

	if (key > n->key)
		return RIGHT;
	else if (key < n->key)
		return LEFT;
	else
		return HERE;
}

// Handles even-parity cases of splay operation only
// Returns direction of X relative to G, post-splay
enum dir child_splay(struct node **g_ptr, unsigned long key)
{
	struct node *g = *g_ptr;

	int p_dir = compare(g, key);
	if (p_dir == NOWHERE)
		return NOWHERE;
	if (p_dir == HERE) {
		// Even parity - G is already X; nothing to do
		return HERE;
	}

	int x_dir = child_splay(&g->child[p_dir], key);
	if (p_dir == NOWHERE)
		return NOWHERE;
	if (x_dir == HERE) {
		// Odd parity - P is now (or was already) X; nothing to do
		return p_dir;
	}

	if (p_dir == x_dir) {
		// Even parity - Zig-zig case
		rotate(g_ptr, p_dir);  // Splay P
		rotate(g_ptr, x_dir);  // Splay X
	} else {
		// Even parity - Zig-zag case
		rotate(&g->child[p_dir], x_dir);  // Splay X
		rotate(g_ptr, p_dir);             // Splay X again
	}
	return HERE;
}

// Handles odd parity case of splay operation only (at the root)
bool splay(struct node **p_ptr, unsigned long key)
{
	int x_dir = child_splay(p_ptr, key);
	if (x_dir == NOWHERE) {
		printf("splay failed\n");
		return false;
	}
	if (x_dir == HERE) {
		// Even parity case - root is now X; nothing to do
		return true;
	}

	// Odd parity - Zig case
	rotate(p_ptr, x_dir); // Splay X
	return true;
}

bool insert(struct node **root_ptr, struct node *x)
{
	if (*root_ptr == NULL) {
		// Trivial case with no root
		*root_ptr = x;
		return true;
	}
	struct node **p_ptr = root_ptr;
	while (*p_ptr != NULL) {
		struct node *p = *p_ptr;
		int x_dir = compare(p, x->key);
		if (x_dir == HERE)
			return false; // Already exists
		p_ptr = &p->child[x_dir];
	}
	*p_ptr = x;
	splay(root_ptr, x->key);
	return true;
}

static inline unsigned long max_key(struct node *n)
{
	while (n->child[RIGHT])
		n = n->child[RIGHT];
	return n->key;
}

bool delete(struct node **root_ptr, unsigned long key)
{
	if (!splay(root_ptr, key))
		return false;
	struct node *old_root = *root_ptr; // (To be removed)

	if (!old_root->child[LEFT]) {
		*root_ptr = old_root->child[RIGHT];
		return true;
	}
	*root_ptr = old_root->child[LEFT];
	splay(root_ptr, max_key(*root_ptr));
	(*root_ptr)->child[RIGHT] = old_root->child[RIGHT];
	return true;
}

// TODO: Customizable node data (maybe take inspiration from BSD headers?)
// TODO: Get/Set/Delete interface
// TODO: More rigorous testing / benchmarking

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

void show_tree(struct node *root)
{
#ifdef SHOW_TREES
	print_node(root, 0, "");
	printf("comparisons: %lu\n", comparisons);
	printf("rotations: %lu\n", rotations);
	comparisons = 0;
	rotations = 0;
	printf("----------------------------------------\n");
#endif
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
	struct node *pool = malloc(num_trials * sizeof(struct node));
	struct node *next_node = pool;

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
			splay(&root, k); \
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

	FREE_NODES();

	// Try out the sequential access theorem experimentally (try `grep comparisons`)
	dur = clock();
	for (int i = 0; i < num_trials; i++) {
		INSERT(i);
	}
	dur = (clock() - dur);
	printf("Average sequential insert duration: %fms\n", (dur / (double)CLOCKS_PER_SEC) / num_trials * 1000.0);

	dur = clock();
	for (int i = 0; i < num_trials; i++) {
		FIND(i);
	}
	dur = (clock() - dur);
	printf("Average sequential find duration: %fms\n", (dur / (double)CLOCKS_PER_SEC) / num_trials * 1000.0);

	// Try out randomized accesses
	dur = clock();
	for (int i = 0; i < num_trials; i++) {
		FIND(rand() % num_trials);
	}
	dur = (clock() - dur);
	printf("Average randomized find duration: %fms\n", (dur / (double)CLOCKS_PER_SEC) / num_trials * 1000.0);

	dur = clock();
	for (int i = 0; i < num_trials; i++) {
		DELETE(i);
	}
	dur = (clock() - dur);
	printf("Average sequential delete duration: %fms\n", (dur / (double)CLOCKS_PER_SEC) / num_trials * 1000.0);

	// Manual testing, compare results online:
	// https://www.cs.usfca.edu/%7Egalles/visualization/SplayTree.html
	FREE_NODES();
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
	printf("This lookup is expected to fail (but not segfault): ");
	FIND(0);

	free(pool);
	return 0;
}
