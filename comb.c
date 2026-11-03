//`which tcc` $CFLAGS -run $0 "$@"; exit $?

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// https://github.com/mkicjn/splay
#include "../splay/src/splay.c"

#define IN(X,T) ((uintptr_t)(X) >= (uintptr_t)(T) \
		&& (uintptr_t)(X) < (uintptr_t)(T) + sizeof(T))


// Terms, memory pool

#define MAX_APPS 10000

struct term {
	struct term *left, *right, *forward;
	struct splay_link link;
} apps[MAX_APPS];
size_t n_apps = 0;

struct term *follow(struct term *t)
{
	// FIXME: Need better cycle detection
	while (IN(t, apps) && t->forward && t->forward != t)
		t = t->forward;
	return t;
}

size_t term_size(struct term *t, bool fwd)
{
	if (fwd)
		t = follow(t);

	if (!IN(t, apps))
		return 1;

	return 1 + term_size(t->left, fwd)
		 + term_size(t->right, fwd);
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
bool reduce_B(struct spine *spine);
bool reduce_C(struct spine *spine);
bool reduce_W(struct spine *spine);

typedef bool (*Comb)(struct spine *spine);

enum comb {
	S, K, I, B, C, W,
	NUM_COMBS
};

const char *comb_names[NUM_COMBS] = {
	[S] = "S",
	[K] = "K",
	[I] = "I",
	[B] = "B",
	[C] = "C",
	[W] = "W",
};

Comb combs[NUM_COMBS] = {
	[S] = reduce_S,
	[K] = reduce_K,
	[I] = reduce_I,
	[B] = reduce_B,
	[C] = reduce_C,
	[W] = reduce_W,
};

bool reduce1(struct term *t, struct spine *spine)
{
	struct spine down;
	t = follow(t);
	down.up = spine;
	down.app = t;

	if (IN(t, apps))
		return reduce1(t->left, &down);
	else if (IN(t, combs))
		return (*(Comb *)t)(spine);
	else
		return false;
}

bool reduce_S(struct spine *spine)
{
	if (spine == NULL || spine->up == NULL || spine->up->up == NULL)
		return false;
	struct term *arg0 = follow(spine->app->right);
	struct term *arg1 = follow(spine->up->app->right);
	struct term *arg2 = follow(spine->up->up->app->right);
	spine->up->up->app->forward = a(a(arg0, arg2), a(arg1, arg2));
	return true;
}

bool reduce_K(struct spine *spine)
{
	if (spine == NULL || spine->up == NULL)
		return false;
	struct term *arg0 = follow(spine->app->right);
	struct term *arg1 = follow(spine->up->app->right);
	spine->up->app->forward = arg0;
	return true;
}

bool reduce_I(struct spine *spine)
{
	if (spine == NULL)
		return false;
	struct term *arg0 = follow(spine->app->right);
	spine->app->forward = arg0;
	return true;
}

bool reduce_B(struct spine *spine)
{
	if (spine == NULL || spine->up == NULL || spine->up->up == NULL)
		return false;
	struct term *arg0 = follow(spine->app->right);
	struct term *arg1 = follow(spine->up->app->right);
	struct term *arg2 = follow(spine->up->up->app->right);
	spine->up->up->app->forward = a(arg0, a(arg1, arg2));
	return true;
}

bool reduce_C(struct spine *spine)
{
	if (spine == NULL || spine->up == NULL || spine->up->up == NULL)
		return false;
	struct term *arg0 = follow(spine->app->right);
	struct term *arg1 = follow(spine->up->app->right);
	struct term *arg2 = follow(spine->up->up->app->right);
	spine->up->up->app->forward = a(a(arg0, arg2), arg1);
	return true;
}

bool reduce_W(struct spine *spine)
{
	if (spine == NULL || spine->up == NULL || spine->up->up == NULL)
		return false;
	struct term *arg0 = follow(spine->app->right);
	struct term *arg1 = follow(spine->up->app->right);
	spine->up->app->forward = a(a(arg0, arg1), arg1);
	return true;
}

size_t reduce(struct term *t, size_t lim)
{
	size_t n = 0;
	while (n < lim && reduce1(t, NULL)) {
		t = follow(t);
		n++;
	}
	return n;
}


// Printing

void print_term(struct term *t, bool fwd)
{
	if (fwd)
		t = follow(t);

	if (IN(t, apps)) {
		print_term(t->left, fwd);
		struct term *right = t->right;
		if (fwd)
			right = follow(right);
		if (IN(right, apps)) {
			printf("(");
			print_term(right, fwd);
			printf(")");
		} else {
			print_term(right, fwd);
		}
	} else if (IN(t, combs)) {
		printf("%s", comb_names[(Comb *)t - combs]);
	} else {
		printf("???");
	}
}

void print_term_idx(struct term *t)
{
	if (IN(t, combs))
		printf("%s", comb_names[(Comb *)t - combs]);
	else
		printf("%zd", t - apps);
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


// Parsing

struct term *parse(const char *str)
{
	static const char *cur = NULL;
	if (str) {
		cur = str;
		return parse(NULL);
	}

	struct term *t = NULL;
	while (*cur != '\0') {
		struct term *next = NULL;
		switch (*(cur++)) {
		case 'S':
		case 's':
			next = (struct term *)&combs[S];
			break;
		case 'K':
		case 'k':
			next = (struct term *)&combs[K];
			break;
		case 'I':
		case 'i':
			next = (struct term *)&combs[I];
			break;
		case 'B':
		case 'b':
			next = (struct term *)&combs[B];
			break;
		case 'C':
		case 'c':
			next = (struct term *)&combs[C];
			break;
		case 'W':
		case 'w':
			next = (struct term *)&combs[W];
			break;
		case '(':
			next = parse(NULL);
			break;
		case ')':
			return t;
		default:
			continue;
		}

		if (t == NULL)
			t = next;
		else
			t = a(t, next);
	}
	return t;
}


// Main

void show_splay(struct splay_link *x, int lvl)
{
	if (x == NULL)
		return;

	struct term *t = SPLAY_CONTAINER(x, struct term, link);
	for (int i = 0; i < lvl; i++)
		printf("|");
	print_term_idx(t);
	printf("\n");
	show_splay(x->child[SPLAY_LEFT], lvl + 1);
	show_splay(x->child[SPLAY_RIGHT], lvl + 1);
}

void try(struct term *t)
{
	printf("----------------------------------------\n");
	print_term(t, false);
	printf("\nSize: %zu\n", term_size(t, false));
	dump_apps();
	printf("\n");
	printf("After %zu head reductions:\n\n", reduce(t, 100));
	print_term(t, true);
	printf("\nSize: %zu\n", term_size(t, true));
	dump_apps();
	printf("\n");
	printf("After attempting to reduce all:\n\n");
	// FIXME: Need proper full reduction
	for (size_t i = 0; i < n_apps; i++)
		reduce(&apps[i], 100);
	print_term(t, true);
	printf("\nSize: %zu\n", term_size(t, true));
	dump_apps();
#ifdef SHOW_SPLAY
	show_splay(root, 0);
#endif
	free_terms();
}

extern const char *stress;
int main()
{
	// Can change SKK to I automatically with:
	// a(a(S, K), K)->forward = I;

	try(parse(stress));

	try(parse("SKK(SKK)"));

	try(parse("SKK(K(SKKI)S)"));

	try(parse("SKK(SII(KKI))"));

	// Ω
	try(parse("SII(SII)"));

	// Calculate 2^4
	try(parse("I(CI)(SBI)(SB(SB(SBI)))(CB)(KI)"));

	return 0;
}


// Stress test

const char *stress = "((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KI)))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KI))))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S(KK))(KI)))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))I))))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KI)))))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KI)))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))I))))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))(KI)))))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))I)))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KI))))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))(KI)))))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KS))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))))((S((S(KS))((S((S(KS))((S(KK))(KS))))((S((S(KS))((S(KK))(KK))))((S(KK))(KK))))))((S((S(KS))((S(KK))(KK))))((S(KK))(KI))))))))";
