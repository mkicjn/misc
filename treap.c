//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

struct treap_link {
	int weight;
	struct treap_link *child[2];
};

enum treap_dir {
	TREAP_LEFT,
	TREAP_RIGHT,
	TREAP_HERE
};

typedef int (*treap_nav_fn)(struct treap_link *, struct treap_link *);

#define CONTAINER(ptr, type, field) \
	(type *)((char *)ptr - offsetof(type, field))

struct treap_link *treap_insert(struct treap_link **root, treap_nav_fn nav, struct treap_link *x)
{
	if (*root == NULL) {
		*root = x;
		x->child[0] = NULL;
		x->child[1] = NULL;
		return NULL;
	}

	struct treap_link *p = *root;
	enum treap_dir dir = nav(x, p);

	if (dir == TREAP_HERE)
		return p;

	struct treap_link *r = treap_insert(&p->child[dir], nav, x);

	x = p->child[dir];
	if (p->weight < x->weight)
		return r;

	p->child[dir] = x->child[1-dir];
	x->child[1-dir] = p;
	*root = x;
	return r;
}


#ifndef NO_MAIN

struct node {
	int value;
	struct treap_link link;
};

int node_nav(struct treap_link *a, struct treap_link *b)
{
	struct node *x = CONTAINER(a, struct node, link);
	struct node *y = CONTAINER(b, struct node, link);

	if (x->value > y->value)
		return TREAP_RIGHT;
	if (x->value < y->value)
		return TREAP_LEFT;
	return TREAP_HERE;
}

int depth_sum = 0;
void print_node_tree(struct treap_link *l, int depth)
{
	if (l == NULL)
		return;

	depth_sum += depth;

	struct node *n = CONTAINER(l, struct node, link);

	print_node_tree(l->child[TREAP_LEFT], depth + 1);

	for (int i = 0; i < depth; i++)
		printf("|");
	printf(" %d\n", n->value);

	print_node_tree(l->child[TREAP_RIGHT], depth + 1);
}

int main(int argc, char **argv)
{
	srand(time(NULL));

	struct treap_link *root = NULL;
	int n = 50;
	if (argc > 1)
		n = atoi(argv[1]);

	for (int i = 0; i < n; i++) {
		struct node *n = malloc(sizeof(*n));
		n->value = i;
		n->link.weight = rand();
		treap_insert(&root, node_nav, &n->link);
	}

	print_node_tree(root, 1);
	printf("Average depth: %f\n", (double)depth_sum / n);

	return 0;
}

#endif
