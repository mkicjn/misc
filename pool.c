//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct thing {
	int val_a;
	double val_b;
	char *str;
};

typedef struct thing val_t;
// ^ TODO: If this will be its own project, it needs a macro like rhmap

union pool_entry {
	val_t value;
	union pool_entry *next;
}; 

struct pool {
	union pool_entry *free;
	union pool_entry *mem;
	struct pool *next;
};

void pool_destroy(struct pool *p)
{
	struct pool *n;
	while (p != NULL) {
		n = p->next;
		free(p->mem);
		free(p);
		p = n;
	}
}

struct pool *pool_new(int num_entries)
{
	struct pool *p = malloc(sizeof(*p));
	union pool_entry *a = malloc(num_entries * sizeof(*a));
	p->mem = a;
	p->free = a;
	p->next = NULL;
	for (int i = 0; i < num_entries-1; i++)
		a[i].next = &a[i+1];
	a[num_entries-1].next = NULL;
	return p;
}

void pool_expand(struct pool *p, int num_entries)
{
	struct pool *new = pool_new(num_entries);
	new->next = p->next;
	p->next = new;
	new->mem[num_entries-1].next = p->free;
	p->free = new->free;
}

val_t *pool_alloc(struct pool *p)
{
	val_t *v = (void *)p->free;
	if (p->free != NULL)
		p->free = p->free->next;
	return v;
}

void pool_free(struct pool *p, val_t *v)
{
	union pool_entry *e = (void *)v;
	e->next = p->free;
	p->free = e;
}

#define POOL_SIZE 1000000

#define OBJ_COUNT 10000000

val_t *arr[OBJ_COUNT];

int main()
{
	struct pool *p = pool_new(100);


	clock_t tick, tock;

	tock = clock();
	for (int i = 0; i < OBJ_COUNT; i++) {
		val_t *v = pool_alloc(p);
		if (v == NULL) {
			pool_expand(p, POOL_SIZE);
			v = pool_alloc(p);
		}
		arr[i] = v;
	}
	tick = clock();
	printf("(pool) Time to alloc: %fs\n", (double)(tick-tock) / CLOCKS_PER_SEC * 1000 / OBJ_COUNT);

	tock = clock();
	for (int i = 0; i < OBJ_COUNT; i++) {
		pool_free(p, arr[i]);
	}
	tick = clock();
	printf("(pool) Time to free: %fs\n", (double)(tick-tock) / CLOCKS_PER_SEC * 1000 / OBJ_COUNT);

	tock = clock();
	for (int i = 0; i < OBJ_COUNT; i++) {
		val_t *v = malloc(sizeof(*v));
		arr[i] = v;
	}
	tick = clock();
	printf("(malloc) Time to alloc: %fs\n", (double)(tick-tock) / CLOCKS_PER_SEC * 1000 / OBJ_COUNT);

	tock = clock();
	for (int i = 0; i < OBJ_COUNT; i++) {
		free(arr[i]);
	}
	tick = clock();
	printf("(malloc) Time to free: %fs\n", (double)(tick-tock) / CLOCKS_PER_SEC * 1000 / OBJ_COUNT);
	
	
	pool_destroy(p);
	return 0;
}
