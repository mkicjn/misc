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

void *car(void *x)
{
	return atom(x) ? NULL : *CAR(x);
}

void *cdr(void *x)
{
	return atom(x) ? NULL : *CDR(x);
}

void print(void *x)
{
	if (atom(x)) {
		printf("%p", x);
	} else {
		printf("(");
		print(car(x));
		for (x = cdr(x); !atom(x); x = cdr(x)) {
			printf(" ");
			print(car(x));
		}
		if (cdr(x)) {
			printf(" . ");
			print(cdr(x));
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

	printf("%p\n", fresh);
	list = list1(0x1);
	print(list);
	printf("\n");
	give(list);

	printf("%p\n", fresh);
	list = list2(0x2, 0x3);
	print(list);
	printf("\n");
	give(list);

	printf("%p\n", fresh);
	list = list3(0x4, 0x5, 0x6);
	print(list);
	printf("\n");
	give(list);

	printf("%p\n", fresh);
	list = list4(0x7, 0x8, 0x9, 0xa);
	print(list);
	printf("\n");
	give(list);

	printf("%p\n", fresh);
	list = list3(0xb, 0xc, 0xd);
	print(list);
	printf("\n");
	give(list);

	printf("%p\n", fresh);
	list = list2(0xe, 0xf);
	print(list);
	printf("\n");
	give(list);

	printf("%p\n", fresh);
	list = list1(0x10);
	print(list);
	printf("\n");
	give(list);

	return 0;
}
