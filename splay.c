#include <stdio.h>
#include <stdbool.h>

struct node {
	int key;
	struct node *child[2];
};

enum dir {
	NOWHERE  = -1,
	LEFT     =  0,
	RIGHT    =  1,
	HERE     =  2,
};

// Move child where p was and swap links around
// e.g., splay1(&g->child[LEFT], RIGHT);
//
//       g             g
//      / \           / \
//     p   d  ->     x   d
//    / \           / \
//   a   x         p   c
//      / \       / \
//     b   c     a   b
//
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
enum dir get_dir(struct node *n, int key)
{
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
		// Odd parity - P is X; nothing to do
		return p_dir;
	}
	struct node *x = p->child[x_dir];

	if (p_dir == x_dir) {
		// Even parity - X is now in the same direction from P as P is from G
		// (Zig-zig case)
		splay1(g_ptr, p_dir); // Splay p
		splay1(g_ptr, x_dir); // Splay x
	} else {
		// Even parity - X is now in the opposite direction from P as P is from G
		// (Zig-zag case)
		splay1(&g->child[p_dir], x_dir); // Splay x
		splay1(g_ptr, p_dir); // Splay x again
	}
	return HERE;
}

// Handles odd parity case of splay operation only (at the root)
bool splay(struct node **g_ptr, int key)
{
	struct node *g = *g_ptr;

	int x_dir = child_splay(g_ptr, key);
	printf("(root) x_dir: %d\n", x_dir);
	if (x_dir == NOWHERE)
		return false;
	if (x_dir == HERE)
		return true;

	// Zig case
	splay1(g_ptr, x_dir); // Splay x
	return true;
}

// TODO: Rigorous testing


bool insert(struct node **root_ptr, struct node *x)
{
	if (*root_ptr == NULL) {
		// Trivial case
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

////////////////////////////////////////////////////////////////////////////////

void show_tree(struct node *n, int depth)
{
	printf("%*s", 8*depth, ""); // Indent
	if (n == NULL) {
		printf("NULL\n");
	} else {
		printf("%d\n", n->key);
		show_tree(n->child[LEFT], depth+1);
		show_tree(n->child[RIGHT], depth+1);
	}
	if (depth == 0)
		printf("--------------------------------------------------------------------------------\n");
}

int main()
{
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

	show_tree(root, 0);
	splay(&root, 1);
	show_tree(root, 0);

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

	show_tree(root, 0);
	splay(&root, 2);
	show_tree(root, 0);

	// https://www.cs.usfca.edu/%7Egalles/visualization/SplayTree.html

	struct node pool[1000];
	struct node *next_node;

#define INSERT(k) \
		do { \
			struct node *n = next_node++; \
			n->key = k; \
			n->child[0] = NULL; \
			n->child[1] = NULL; \
			insert(&root, n); \
			show_tree(root, 0); \
		} while (0)

#define FIND(k) \
		do { \
			splay(&root, k); \
			show_tree(root, 0); \
		} while (0)

#define FREE_NODES() \
		do { \
			root = NULL; \
			next_node = pool; \
		} while (0)


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

	return 0;
}
