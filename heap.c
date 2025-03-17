#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>

#define SWAP(x, y) \
	do { \
		typeof(x) z = x; \
		x = y; \
		y = z; \
	} while (0)

#define PARENT(x)  (((x) - 1) >> 1)
#define LEFT(x)  (((x) << 1) + 1)
#define RIGHT(x) (((x) + 1) << 1)
#define BEST(h, n, x) \
	(LEFT(x) >= n ? x \
	 : RIGHT(x) >= n ? LEFT(x) \
	 : h[LEFT(x)] < h[RIGHT(x)] ? LEFT(x) \
	 : RIGHT(x))

void sift_up(long *h, long i)
{
	long x = PARENT(i);
	if (h[i] < h[x]) {
		SWAP(h[i], h[x]);
		sift_up(h, x);
	}
}

void sift_down(long *h, long n, long i)
{
	long x = BEST(h, n, i);
	if (h[x] < h[i]) {
		SWAP(h[x], h[i]);
		sift_down(h, n, x);
	}
}

void heapify(long *h, long n)
{
	for (long i = n - 1; i >= 0; i--)
		sift_down(h, n, i);
}

void heapsort(long *h, long n)
{
	heapify(h, n);
	for (long i = n - 1; i >= 0; i--) {
		SWAP(h[0], h[i]);
		sift_down(h, i, 0);
	}
}

void heap_push(long *h, long n, long x)
{
	h[n] = x;
	sift_up(h, n);
}

long heap_pop(long *h, long n)
{
	long x = h[0];
	h[0] = h[--n];
	sift_down(h, n, 0);
	return x;
}


#ifndef NO_MAIN

#ifndef SIZE
#define SIZE 20
#endif

long test[SIZE+1];

int main()
{
	long size = SIZE;
	srand(time(NULL));
	for (int i = 0; i < size; i++)
		test[i] = rand() % SIZE;

	for (long i = 0; i < size; i++) {
		printf("%ld ", test[i]);
	}
	printf("\n");

	heapify(test, size);
	for (long i = 0; i < size; i++) {
		printf("%ld ", test[i]);
	}
	printf("\n");

	for (long i = 0; i < SIZE; i++)
		heap_push(test, size++, rand() % SIZE);
	for (long i = 0; i < size; i++) {
		printf("%ld ", test[i]);
	}
	printf("\n");

	heap_pop(test, size--);
	for (long i = 0; i < size; i++) {
		printf("%ld ", test[i]);
	}
	printf("\n");

	heapsort(test, size);
	for (long i = 0; i < size; i++) {
		printf("%ld ", test[i]);
	}
	printf("\n");
}
#endif
