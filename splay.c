#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

struct node {
	int key;
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

// Move x where p was and swap links around
// e.g., splay1(&g->child[LEFT], RIGHT);
//
//      (g)           (g)
//      / \           / \
//     p   d   ->    x   d
//    / \           / \
//   a   x         p   c
//      / \       / \
//     b   c     a   b
//
// All types of splays have this in common
void splay1(struct node **p_ptr, int x_dir)
{
	struct node *p = *p_ptr;
	struct node *x = p->child[x_dir];
	p->child[x_dir] = x->child[1-x_dir];
	x->child[1-x_dir] = p;
	*p_ptr = x;
}

// Local search
// Returns direction of X
int comparisons = 0;
enum dir get_dir(struct node *n, int key)
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
enum dir child_splay(struct node **g_ptr, int key)
{
	struct node *g = *g_ptr;

	int p_dir = get_dir(g, key);
	if (p_dir == NOWHERE)
		return NOWHERE;
	printf("p_dir: %d\n", p_dir);
	if (p_dir == HERE) {
		// Even parity - G is already X; nothing to do
		return HERE;
	}
	struct node *p = g->child[p_dir];

	int x_dir = child_splay(&g->child[p_dir], key);
	printf("x_dir: %d\n", x_dir);
	if (p_dir == NOWHERE)
		return NOWHERE;
	if (x_dir == HERE) {
		// Odd parity - P is now (or was already) X; nothing to do
		return p_dir;
	}
	struct node *x = p->child[x_dir];

	if (p_dir == x_dir) {
		// Even parity - Zig-zig case
		splay1(g_ptr, p_dir);  // Splay P
		splay1(g_ptr, x_dir);  // Splay X
	} else {
		// Even parity - Zig-zag case
		splay1(&g->child[p_dir], x_dir);  // Splay X
		splay1(g_ptr, p_dir);             // Splay X again
	}
	return HERE;
}

// Handles odd parity case of splay operation only (at the root)
bool splay(struct node **p_ptr, int key)
{
	struct node *p = *p_ptr;

	int x_dir = child_splay(p_ptr, key);
	printf("(root) x_dir: %d\n", x_dir);
	if (x_dir == NOWHERE)
		return false;
	if (x_dir == HERE) {
		// Even parity case - root is now X; nothing to do
		return true;
	}

	// Odd parity - Zig case
	splay1(p_ptr, x_dir); // Splay X
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
		int x_dir = get_dir(p, x->key);
		if (x_dir == HERE)
			return false; // Already exists
		p_ptr = &p->child[x_dir];
	}
	*p_ptr = x;
	splay(root_ptr, x->key);
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
	printf("%s%d\n", s, n->key);
	print_node(n->child[LEFT], depth+1, "L: ");
	print_node(n->child[RIGHT], depth+1, "R: ");

	if (depth == 0) {
		printf("comparisons: %d\n", comparisons);
		comparisons = 0;
		printf("----------------------------------------\n");
	}
}

void show_tree(struct node *root)
{
	print_node(root, 0, "Root: ");
}

int main()
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

	// Dynamic testing
	static struct node pool[1000];
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

#define FREE_NODES() \
		do { \
			root = NULL; \
			next_node = pool; \
		} while (0)


	// Try out the sequential access theorem experimentally (try `grep comparisons`)
	FREE_NODES();
	srand(time(NULL));
	for (int i = 0; i < 100; i++) {
		INSERT(i);
	}
	for (int i = 0; i < 100; i++) {
		FIND(i);
	}

	// Try out randomized accesses
	for (int i = 0; i < 100; i++) {
		FIND(rand() % 100);
	}

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

	return 0;
}
