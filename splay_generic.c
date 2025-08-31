//$(which tcc) $CFLAGS -run $0 "$@"; exit $?

// Generic splay tree API experiment

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

struct splay_link {
	struct splay_link *child[2];
};

enum splay_dir {
	// (used for child indexing)
	LEFT  = 0,
	RIGHT = 1,
	// (not used for child indexing)
	HERE,
	NOWHERE,
};

typedef enum splay_dir (*splay_nav_fn)(const struct splay_link *n, const void *nav_arg);

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
void rotate(struct splay_link **p_ptr, int x_dir)
{
	struct splay_link *p = *p_ptr;
	struct splay_link *x = p->child[x_dir];
	p->child[x_dir] = x->child[1-x_dir];
	x->child[1-x_dir] = p;
	*p_ptr = x;
}

// Top-down strategy
bool splay(struct splay_link **root_ptr, splay_nav_fn nav, const void *nav_arg)
{
	if (*root_ptr == NULL)
		return false;

	struct splay_link *subtree[2] = {NULL, NULL};
	struct splay_link **leaf[2] = {&subtree[0], &subtree[1]};

	for (;;) {
		struct splay_link *g = *root_ptr;
		enum splay_dir dir = nav(g, nav_arg);
		if (dir == HERE)
			break;

		struct splay_link *p = g->child[dir];
		if (nav(p, nav_arg) == dir) {
			g = p;
			rotate(root_ptr, dir);
		}
		// This check comes after because rotation could put NULL in its place
		if (g->child[dir] == NULL)
			break;

		*leaf[1-dir] = g;
		leaf[1-dir] = &g->child[dir];
		*root_ptr = g->child[dir];
	}

	struct splay_link *g = *root_ptr;
	*leaf[LEFT] = g->child[LEFT];
	*leaf[RIGHT] = g->child[RIGHT];
	g->child[LEFT] = subtree[LEFT];
	g->child[RIGHT] = subtree[RIGHT];
	return nav(g, nav_arg) == HERE;
}

bool find(struct splay_link **root_ptr, splay_nav_fn nav, const void *nav_arg)
{
	if (!splay(root_ptr, nav, nav_arg)) {
		printf("lookup failed\n");
		return false;
	} else {
		return true;
	}
}

bool insert(struct splay_link **root_ptr, splay_nav_fn nav, struct splay_link *x)
{
	if (*root_ptr == NULL) {
		*root_ptr = x;
		return true;
	}

	if (splay(root_ptr, nav, x))
		return false;

	struct splay_link *g = *root_ptr;
	int dir = nav(g, x);
	x->child[1-dir] = g;
	x->child[dir] = g->child[dir];
	g->child[dir] = NULL;
	*root_ptr = x;
	return true;
}

enum splay_dir splay_nav_to_min(const struct splay_link *n, const void *arg)
{
	(void)arg;
	if (n == NULL)
		return NOWHERE;
	else if (n->child[LEFT] == NULL)
		return HERE;
	else
		return LEFT;
}

bool delete(struct splay_link **root_ptr, splay_nav_fn nav, const void *nav_arg)
{
	if (!splay(root_ptr, nav, nav_arg))
		return false;
	struct splay_link *del = *root_ptr;

	if (!del->child[RIGHT]) {
		*root_ptr = del->child[LEFT];
		return true;
	} else if (!del->child[LEFT]) {
		*root_ptr = del->child[RIGHT];
		return true;
	}
	*root_ptr = del->child[RIGHT];
	splay(root_ptr, splay_nav_to_min, NULL);
	(*root_ptr)->child[LEFT] = del->child[LEFT];
	return true;
}

// (Reimplemented here for fun)
#define OFFSET_OF(T, MEMB) ((size_t)(&((T *)0)->MEMB))
#define CONTAINER_OF(PTR, T, MEMB) ((T *)((char *)(PTR) - OFFSET_OF(T, MEMB)))


// ******************** No longer generic ********************

struct node {
	unsigned long key;
	struct splay_link link;
};

enum splay_dir node_nav(const struct splay_link *l, const void *arg)
{
	if (l == NULL)
		return NOWHERE;

	const struct node *n1 = CONTAINER_OF(l, struct node, link);
	const struct node *n2 = CONTAINER_OF(arg, struct node, link);
	if (n2->key > n1->key)
		return RIGHT;
	else if (n2->key < n1->key)
		return LEFT;
	else
		return HERE;
}

void print_node(struct splay_link *l, int depth, const char *s)
{
	if (l == NULL)
		return;

	struct node *n = CONTAINER_OF(l, struct node, link);
	for (int i = 1; i < depth; i++)
		printf("| ");
	printf("%s%lu\n", s, n->key);

	print_node(l->child[LEFT], depth+1, "L:");
	print_node(l->child[RIGHT], depth+1, "R:");
}

void show_tree(struct splay_link *l)
{
#ifdef SHOW_TREES
	print_node(l, 0, "");
	printf("----------------------------------------\n");
#else
	(void)l;
#endif
}

int main(int argc, char **argv)
{
	// Base case testing
#define NODE(X, L, R) &(struct node){.key=(X), .link=(struct splay_link){.child={(L), (R)}}}.link
#define LEAF(X) NODE(X, NULL, NULL)
#define FIND(k) \
		do { \
			struct node arg = {.key = k}; \
			find(&root, node_nav, &arg.link); \
			show_tree(root); \
		} while (0)

	struct splay_link *root;

	root = 
		NODE(3,
			NODE(2,
				NODE(1,
					NULL,
					NULL),
				NULL),
			NULL);

	show_tree(root);
	FIND(1);
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
	FIND(2);
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
	FIND(1);
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
	FIND(5);
	show_tree(root);

	// Dynamic testing
	size_t num_trials = 1000;
	if (argc > 1)
		num_trials = atol(argv[1]);

#define INSERT(k) \
		do { \
			struct node *arg = next_node++; \
			arg->key = k; \
			arg->link.child[LEFT] = NULL; \
			arg->link.child[RIGHT] = NULL; \
			insert(&root, node_nav, &arg->link); \
			show_tree(root); \
		} while (0)

#define DELETE(k) \
		do { \
			struct node arg = {.key = k}; \
			delete(&root, node_nav, &arg); \
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

	// Release node pool memory
	free(pool);
	pool = NULL;
	FREE_NODES();

	return 0;
}
