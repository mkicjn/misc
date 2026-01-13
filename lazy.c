//usr/bin/env tcc $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdbool.h>

#define COUNT(X) (sizeof(X)/sizeof(X[0]))
#define LAST(X) ((X) + COUNT(X))
#define IN(X, A) ((void *)(A) <= (X) && (X) < (void *)LAST(A))

#define CELL_PTRS 3
#define CAR(X) ((void **)X)
#define CDR(X) ((void **)X+1)
#define FREE(X) ((void **)X+2)

void *cells[CELL_PTRS << 10];

static inline bool atom(void *x)
{
	return !IN(x, cells);
}


// Lazy deallocation - a novel strategy?

void **fresh = cells;
void *next = NULL;

void give(void *x)
{
	if (!atom(x)) {
		*FREE(x) = next;
		next = x;
	}
}

void *take(void)
{
	void *ret = NULL;
	if (!next) {
		ret = fresh;
		fresh += CELL_PTRS;
	} else {
		ret = next;
		next = *FREE(next);
		give(*CAR(ret));
		give(*CDR(ret));
	}
	return ret;
}


// Some basic primitives for testing

void *cons(void *x, void *y)
{
	void *cell = take();
	*CAR(cell) = x;
	*CDR(cell) = y;
	return cell;
}

void decons(void *x, void **a, void **d)
{
	if (!atom(x)) {
		*a = *CAR(x);
		*d = *CDR(x);
		*CAR(x) = NULL;
		*CDR(x) = NULL;
	} else {
		*a = NULL;
		*d = NULL;
	}
	give(x);
}

void print(void *x);

void *pbody(void *x)
{
	if (atom(x)) {
		return x;
	} else {
		void *a, *d;
		decons(x, &a, &d);
		print(a);
		if (d)
			printf(" ");
		return pbody(d);
	}
}

void print(void *x)
{
	if (!x) {
		printf("()");
	} else if (atom(x)) {
		printf("%p", x);
	} else {
		printf("(");
		void *d = pbody(x);
		if (d) {
			printf(". ");
			print(d);
		}
		printf(")");
	}
}


// Messing with the primitives

#define list0(...) NULL
#define list1(x, ...) cons((void *)(x), list0(__VA_ARGS__))
#define list2(x, ...) cons((void *)(x), list1(__VA_ARGS__))
#define list3(x, ...) cons((void *)(x), list2(__VA_ARGS__))
#define list4(x, ...) cons((void *)(x), list3(__VA_ARGS__))

int main()
{
	void *list, *list2;

	printf("%llu\n", (fresh - cells) / CELL_PTRS);
	list = list1(0x1);
	print(list);
	printf("\n");

	printf("%llu\n", (fresh - cells) / CELL_PTRS);
	list = list2(0x2, 0x3);
	print(list);
	printf("\n");

	printf("%llu\n", (fresh - cells) / CELL_PTRS);
	list = list3(0x4, 0x5, 0x6);
	print(list);
	printf("\n");

	printf("%llu\n", (fresh - cells) / CELL_PTRS);
	list = list4(0x7, 0x8, 0x9, 0xa);
	print(list);
	printf("\n");

	printf("%llu\n", (fresh - cells) / CELL_PTRS);
	list = list3(0xb, 0xc, 0xd);
	print(list);
	printf("\n");

	printf("%llu\n", (fresh - cells) / CELL_PTRS);
	list = list2(0xe, 0xf);
	print(list);
	printf("\n");

	printf("%llu\n", (fresh - cells) / CELL_PTRS);
	list = list1(0x10);
	print(list);
	printf("\n");

	return 0;
}
