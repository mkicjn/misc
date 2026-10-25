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
		print_term(t->as.fwd.dest);
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

bool reduce(struct term **tp)
{
	struct term *t = *tp;
	struct term *prev[3];
	prev[0] = NULL;
	prev[1] = NULL;
	prev[2] = NULL;
	bool done = false;
	while (!done) {
		switch (t->type) {
		case App:
			prev[2] = prev[1];
			prev[1] = prev[0];
			prev[0] = t;
			t = t->as.app.left;
			break;
		case Fwd:
			t = t->as.fwd.dest;
			break;
		default:
			done = true;
		}
	}

	switch (t->type) {
	case S:
		if (prev[2] == NULL)
			break;
		*tp = app(app(prev[0]->as.app.right, prev[2]->as.app.right),
		          app(prev[1]->as.app.right, prev[2]->as.app.right));
		t->type = Fwd;
		t->as.fwd.dest = *tp;
		return true;
	case K:
		if (prev[1] == NULL)
			break;
		t->type = Fwd;
		t->as.fwd.dest = prev[0]->as.app.right;
		*tp = t;
		return true;
	case I:
		if (prev[0] == NULL)
			break;
		t->type = Fwd;
		t->as.fwd.dest = prev[0]->as.app.right;
		*tp = t;
		return true;
	}
	return false;
}

int main()
{
	//struct term *t = app(i(), app(app(k(), s()), i()));
	struct term *t = app(app(app(s(), k()), k()),
			     app(app(k(), app(i(), i())), s()));
	do {
		// Reduction here is until WHNF (I think)
		print_term(t);
		printf("\n");
	} while (reduce(&t));
	return 0;
}
