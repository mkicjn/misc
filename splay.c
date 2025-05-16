#include <stdio.h>

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

// Even-parity recursive case of splay operation
// Returns direction of X relative to G, post-splay
enum dir child_splay(struct node **g_ptr, int key)
{
	struct node *g = *g_ptr;

	int p_dir = get_dir(g, key);
	if (p_dir == NOWHERE)
		return NOWHERE;
	printf("p_dir: %d\n", p_dir);
	if (p_dir == HERE) {
		// G is X
		return HERE;
	}
	struct node *p = g->child[p_dir];

	int x_dir = get_dir(p, key); //child_splay(&g->child[p_dir], key) // ???
	printf("x_dir: %d\n", x_dir);
	if (p_dir == NOWHERE)
		return NOWHERE;
	if (x_dir == HERE) {
		// P is X
		return p_dir;
	}
	struct node *x = p->child[x_dir];

	if (p_dir == x_dir) {
		// Zig-zig case
		splay1(g_ptr, p_dir); // Splay p
		splay1(g_ptr, x_dir); // Splay x
	} else {
		// Zig-zag case
		splay1(&g->child[p_dir], x_dir); // Splay x
		splay1(g_ptr, p_dir); // Splay x again
	}
	return HERE;
}

// TODO: Handle odd parity case (root)


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
}

int main()
{
	struct node n_x, n_p, n_g;
	struct node *g;

	n_x = (struct node){.key = 1, .child = {NULL, NULL}};
	n_p = (struct node){.key = 2, .child = {&n_x, NULL}};
	n_g = (struct node){.key = 3, .child = {&n_p, NULL}};

	g = &n_g;
	show_tree(g, 0);
	child_splay(&g, n_x.key);
	show_tree(g, 0);


	n_x = (struct node){.key = 2, .child = {NULL, NULL}};
	n_p = (struct node){.key = 1, .child = {NULL, &n_x}};
	n_g = (struct node){.key = 3, .child = {&n_p, NULL}};

	g = &n_g;
	show_tree(g, 0);
	child_splay(&g, n_x.key);
	show_tree(g, 0);

}
