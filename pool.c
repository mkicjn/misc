#include <stdio.h>
#include <stdlib.h>

typedef unsigned long val_t;
// ^ TODO: If this will be its own project, it needs a macro like rhmap

union pool_entry {
	val_t value;
	union pool_entry *next;
}; 

struct pool {
	union pool_entry *free;
	union pool_entry *mem;
};

void pool_destroy(struct pool *p)
{
	free(p->mem);
	free(p);
}

struct pool *pool_new(int num_entries)
{
	struct pool *p = malloc(sizeof(*p));
	union pool_entry *a = malloc(num_entries * sizeof(*a));
	p->mem = a;
	p->free = a;
	for (int i = 0; i < num_entries-1; i++)
		a[i].next = &a[i+1];
	a[num_entries-1].next = NULL;
	return p;
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

int main()
{
	struct pool *p = pool_new(100);

	unsigned long *ptr = pool_alloc(p);
	pool_free(p, ptr);
	// TODO: Write a benchmark test

	pool_destroy(p);
	return 0;
}
