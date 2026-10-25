//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

struct node {
	enum {
		LEAF,
		BRANCH
	} type;
	union {
		struct {
			uint32_t val;
		} leaf;
		struct {
			struct node *left;
			struct node *right;
		} branch;
	} as;
} nodes[10000];
size_t n_nodes = 0;

uint64_t hash_node(struct node *n)
{
	switch (n->type) {
	case LEAF:
		return ((uint64_t)n->as.leaf.val << 32) | 0xffffffff;
	case BRANCH:
		return ((uint64_t)(n->as.branch.left - nodes) << 32)
			| ((uint64_t)(n->as.branch.right - nodes));
	default:
		return ~(uint64_t)0;
	}
}

void print_node(struct node *n)
{
	switch (n->type) {
	case LEAF:
		printf("%" PRIu32, n->as.leaf.val);
		break;
	case BRANCH:
		printf("(");
		print_node(n->as.branch.left);
		printf(" ");
		print_node(n->as.branch.right);
		printf(")");
	}
}

bool node_eq(struct node *a, struct node *b)
{
	if (a->type != b->type)
		return false;
	switch (a->type) {
	case LEAF:
		return a->as.leaf.val == b->as.leaf.val;
	case BRANCH:
		return (a->as.branch.left == b->as.branch.left)
			&& (a->as.branch.right == b->as.branch.right);
	default:
		return false;
	}
}

void dump_nodes(void)
{
	for (int i = 0; i < n_nodes; i++) {
		printf("%d: ", i);
		switch (nodes[i].type) {
		case LEAF:
			printf("l(%d)\n", nodes[i].as.leaf.val);
			break;
		case BRANCH:
			printf("b(%d,%d)\n",
					nodes[i].as.branch.left - nodes,
					nodes[i].as.branch.right - nodes);
			break;
		default:
			printf("???\n");
		}
	}
}

#define HASHTAB_SIZE 161803
struct node *hashtab[HASHTAB_SIZE] = {0};
struct node *intern(struct node *n)
{
	struct node **t = &hashtab[hash_node(n) % HASHTAB_SIZE];
	if (*t != NULL) {
		// FIXME: Collisions will cause errors
		return *t;
	} else {
		struct node *new = &nodes[n_nodes++];
		*new = *n;
		*t = new;
		return new;
	}
}

struct node *b(struct node *l, struct node *r)
{
	struct node n;
	n.type = BRANCH;
	n.as.branch.left = l;
	n.as.branch.right = r;
	return intern(&n);
}

struct node *l(uint32_t v)
{
	struct node n;
	n.type = LEAF;
	n.as.leaf.val = v;
	return intern(&n);
}

int main()
{
	struct node *n = b(b(l(1), l(2)), b(l(1), l(2)));
	print_node(n);
	printf("\n");
	dump_nodes();

	n_nodes = 0;
	memset(hashtab, 0, sizeof(hashtab));

	struct node *skk = b(b(l(1), l(2)), l(2));
	struct node *t = b(skk, b(b(l(2), b(skk, l(3))), l(1)));
	print_node(t);
	printf("\n");
	dump_nodes();

	return 0;
}
