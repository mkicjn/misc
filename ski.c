//`which tcc` $CFLAGS -run $0 "$@"; exit $?

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// https://github.com/mkicjn/splay
#include "../splay/src/splay.c"


// Terms, memory pool, and printing

#define MAX_TERMS 10000

struct term {
	struct term *left, *right, *forward;
	struct splay_link link;
};

struct term *S = (struct term *)-1;
struct term *K = (struct term *)-2;
struct term *I = (struct term *)-3;

struct term apps[MAX_TERMS];
size_t n_apps = 0;

static inline bool is_app(struct term *t)
{
	return (uintptr_t)apps <= (uintptr_t)(t)
		&& (uintptr_t)t < (uintptr_t)(&apps[MAX_TERMS]);
}

struct term *follow(struct term *t)
{
	while (is_app(t) && t->forward)
		t = t->forward;
	return t;
}

size_t term_size(struct term *t, bool fwd)
{
	if (fwd)
		t = follow(t);

	if (!is_app(t))
		return 1;

	return 1 + term_size(t->left, fwd)
		 + term_size(t->right, fwd);
}

void print_term(struct term *t, bool fwd)
{
	if (fwd)
		t = follow(t);

	if (is_app(t)) {
		printf("(");
		print_term(t->left, fwd);
		printf(" ");
		print_term(t->right, fwd);
		printf(")");
	} else if (t == S) {
		printf("S");
	} else if (t == K) {
		printf("K");
	} else if (t == I) {
		printf("I");
	} else {
		printf("???");
	}
}

void print_term_idx(struct term *t)
{
	if (t == S) {
		printf("S");
	} else if (t == K) {
		printf("K");
	} else if (t == I) {
		printf("I");
	} else {
		printf("%zd", t - apps);
	}
}

void dump_apps(void)
{
	for (size_t i = 0; i < n_apps; i++) {
		printf("%zu: ", i);
		printf("(");
		print_term_idx(apps[i].left);
		printf(" ");
		print_term_idx(apps[i].right);
		printf(")");
		if (apps[i].forward) {
			printf(" -> ");
			print_term_idx(apps[i].forward);
		}
		printf("\n");
	}
}


// Term construction, memory sharing

enum splay_dir term_nav(const struct splay_link *xl, const struct splay_link *yl)
{
	const struct term *x = SPLAY_CONTAINER(xl, struct term, link);
	const struct term *y = SPLAY_CONTAINER(yl, struct term, link);

	if (x->left < y->left)
		return SPLAY_LEFT;
	if (x->left > y->left)
		return SPLAY_RIGHT;
	if (x->right < y->right)
		return SPLAY_LEFT;
	if (x->right > y->right)
		return SPLAY_RIGHT;
	return SPLAY_HERE;
}

struct splay_link *root = NULL;
struct term *a(struct term *x, struct term *y) // apply
{
	struct term *app = &apps[n_apps];
	app->left = x;
	app->right = y;
	app->forward = NULL;
	
	struct splay_link *existing = splay_insert(&root, term_nav, &app->link);
	if (existing == NULL) {
		n_apps++;
		return app;
	} else {
		return follow(SPLAY_CONTAINER(existing, struct term, link));
	};
}

void free_terms(void)
{
	root = NULL;
	n_apps = 0;
}


// Reduction

struct spine {
	struct spine *up;
	struct term *app;
};

bool reduce_S(struct spine *spine);
bool reduce_K(struct spine *spine);
bool reduce_I(struct spine *spine);

bool reduce1(struct term *t, struct spine *spine)
{
	struct spine down;
	t = follow(t);
	down.up = spine;
	down.app = t;

	if (t == S)
		return reduce_S(spine);
	else if (t == K)
		return reduce_K(spine);
	else if (t == I)
		return reduce_I(spine);
	else
		return reduce1(t->left, &down);
}

bool reduce_S(struct spine *spine)
{
	if (spine == NULL || spine->up == NULL || spine->up->up == NULL)
		return false;
	struct term *arg0 = spine->app->right;
	struct term *arg1 = spine->up->app->right;
	struct term *arg2 = spine->up->up->app->right;
	spine->up->up->app->forward = a(a(arg0, arg2), a(arg1, arg2));
	return true;
}

bool reduce_K(struct spine *spine)
{
	if (spine == NULL || spine->up == NULL)
		return false;
	struct term *arg0 = spine->app->right;
	struct term *arg1 = spine->up->app->right;
	spine->up->app->forward = arg0;
	return true;
}

bool reduce_I(struct spine *spine)
{
	if (spine == NULL)
		return false;
	struct term *arg0 = spine->app->right;
	spine->app->forward = arg0;
	return true;
}

size_t reduce(struct term *t)
{
	size_t n = 0;
	while (reduce1(t, NULL)) {
		t = follow(t);
		n++;
	}
	return n;
}


// Main

void try(struct term *t)
{
	printf("----------------------------------------\n");
	print_term(t, false);
	printf("\nSize: %zu\n", term_size(t, false));
	dump_apps();
	printf("\n");
	printf("Reduced to WHNF in %zu steps:\n\n", reduce(t));
	print_term(t, true);
	printf("\nSize: %zu\n", term_size(t, true));
	dump_apps();
	free_terms();
}

struct term *stress(void);
int main()
{
	// a(a(S, K), K)->forward = I;

	try(stress());

	try(a(a(a(S, K), K), a(a(S, K), K)));

	try(a(a(a(S, K), K), a(a(K, a(a(a(S, K), K), I)), S)));

	try(a(a(a(S, K), K), a(a(a(S, I), I), a(a(K, K), I))));

	return 0;
}


// Stress test function

struct term *stress(void)
{
	return
a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, I))))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, I)))))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(K, K)), a(K, I))))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), I)))))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, I))))))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, I))))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), I)))))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(K, I))))))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), I))))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, I)))))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(K, I))))))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, S)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, S)))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, K)))))), a(a(S, a(a(S, a(K, S)), a(a(S, a(K, K)), a(K, K)))), a(a(S, a(K, K)), a(K, I))))))));
}
