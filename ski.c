//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdbool.h>

struct term {
	enum {
		S, K, I, App, Fwd
	} type;
	union {
		struct {
			struct term *left, *right;
		} app;
		struct {
			struct term *dest;
		} fwd;
	} as;
} terms[10000];
size_t next_term = 0;

void print_term(struct term *t)
{
	switch (t->type) {
	case S:
		printf("S");
		break;
	case K:
		printf("K");
		break;
	case I:
		printf("I");
		break;
	case App:
		printf("(");
		print_term(t->as.app.left);
		printf(" ");
		print_term(t->as.app.right);
		printf(")");
		break;
	case Fwd:
		printf("->");
		print_term(t->as.fwd.dest);
		break;
	default:
		printf("{type %d}", t->type);
		break;
	}
}

struct term *get(void)
{
	return &terms[next_term++];
}

struct term *s(void)
{
	struct term *t = get();
	t->type = S;
	return t;
}

struct term *k(void)
{
	struct term *t = get();
	t->type = K;
	return t;
}

struct term *i(void)
{
	struct term *t = get();
	t->type = I;
	return t;
}

struct term *app(struct term *l, struct term *r)
{
	struct term *t = get();
	t->type = App;
	t->as.app.left = l;
	t->as.app.right = r;
	return t;
}

bool reduce(struct term *t)
{
	struct term *apps[10]; // FIXME: Stack limit
	size_t d = 0; // "depth"

	for (;;) {
		if (t->type == Fwd) {
			t = t->as.fwd.dest;
		} else if (t->type == App) {
			apps[d++] = t;
			t = t->as.app.left;
		} else {
			break;
		}
	}

#ifdef DEBUG
	for (int i = 0; i < d; i++) {
		printf("Level %d: ", i);
		print_term(apps[i]);
		printf("\n");
	}
	printf("Combinator (level %d): ", d);
	print_term(t);
	printf("\n");
#endif

	struct term *arg0 = d > 0 ? apps[d - 1]->as.app.right : NULL;
	struct term *arg1 = d > 1 ? apps[d - 2]->as.app.right : NULL;
	struct term *arg2 = d > 2 ? apps[d - 3]->as.app.right : NULL;
	struct term *top = NULL;
	switch (t->type) {
	case S:
		if (arg2 == NULL)
			return false;
#ifdef DEBUG
		printf("Apply S to ");
		print_term(arg0);
		printf(", ");
		print_term(arg1);
		printf(", ");
		print_term(arg2);
		printf("\n");
#endif
		top = apps[d - 3];
		top->type = Fwd;
		top->as.fwd.dest = app(app(arg0, arg2), app(arg1, arg2));
		return true;
	case K:
		if (arg1 == NULL)
			return false;
#ifdef DEBUG
		printf("Apply K to ");
		print_term(arg0);
		printf(", ");
		print_term(arg1);
		printf("\n");
#endif
		top = apps[d - 2];
		*top = *arg0;
		return true;
	case I:
		if (arg0 == NULL)
			return false;
#ifdef DEBUG
		printf("Apply I to ");
		print_term(arg0);
		printf("\n");
#endif
		top = apps[d - 1];
		*top = *arg0;
		return true;
	}
	return false;
}

int main()
{
	// TODO: Implement hash consing?
	struct term *skk = app(app(s(), k()), k());
	struct term *t = app(skk, app(app(k(), app(skk, i())), s()));
	//struct term *t = app(skk, skk);
	do {
		// Reduction here is until WHNF (I think)
		print_term(t);
		printf("\n");
	} while (reduce(t));
	return 0;
}
