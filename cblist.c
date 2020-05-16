#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef unsigned long key_t;
typedef unsigned long val_t;

struct entry {
	key_t key;
	val_t val;
};

struct cblist {
	struct entry *mem;
	size_t mask; // Size mask, i.e. 2^N-1
	size_t head; // Index of first occupied position
	size_t tail; // Index after last occupied position
};

bool cbl_seek(struct cblist *l, key_t k)
{ // Looks for k and moves it to list head; returns success
	register size_t mask = l->mask;
	if (l->mem[l->head].key == k)
		return true;
	size_t i = (l->head + 1) & l->mask;
	while (i != l->tail) {
		if (l->mem[i].key == k) {
			// Duplicate search key's entry to head
			l->head = (l->head - 1) & mask;
			l->mem[l->head] = l->mem[i];
			// Replace with last entry in list
			l->tail = (l->tail - 1) & mask;
			l->mem[i] = l->mem[l->tail];
			return true;
		}
		i = (i + 1) & mask;
	}
	return false;
}

bool cbl_full(struct cblist *l)
{ // Returns true if l has no more free space
	return l->head == ((l->tail + 1) & l->mask);
}

val_t cbl_set(struct cblist *l, key_t k, val_t v)
{ // Sets k to v in l, making new entry if necessary; returns previous value or 0
	val_t prev = (val_t)0;
	if (cbl_seek(l, k)) {
		prev = l->mem[l->head].val;
		l->mem[l->head].val = v;
	} else if (!cbl_full(l)) {
		l->head = (l->head - 1) & l->mask;
		l->mem[l->head].key = k;
		l->mem[l->head].val = v;
	}
	return prev;
}

bool cbl_found;
val_t cbl_get(struct cblist *l, key_t k)
{ // Returns value of k in l; sets cbl_found appropriately
	if ((cbl_found = cbl_seek(l, k)))
		return l->mem[l->head].val;
	else
		return (val_t)0;
}

val_t cbl_del(struct cblist *l, key_t k)
{ // Removes entry for k from l, returns its value or 0
	val_t prev = (val_t)0;
	if (cbl_seek(l, k)) {
		prev = l->mem[l->head].val;
		l->head = (l->head + 1) & l->mask;
	}
	return prev;
}

void cbl_init(struct cblist *l, struct entry *m, size_t s)
{
	s /= sizeof(struct entry);
	s >>= 1;
	s = (1 << ffs(s)) - 1;

	l->mem = m;
	l->head = 0;
	l->tail = 0;
	l->mask = s;
}

/*
 *		Begin test program
 */

key_t hash(char *s)
{
	key_t key = 0;
	for (char c = *s; c; c = *(++s)) {
		key ^= c;
		key += (key << 4);
	}
	return key;
}

void cbl_print(struct cblist *l)
{
	for (size_t i = 0; i <= l->mask; i++) {
		printf("%lu\t->\t%lu", l->mem[i].key, l->mem[i].val);
		if (l->head == i)
			printf("\t(head)");
		if (l->tail == i)
			printf("\t(tail)");
		printf("\n");
	}
	printf("\n\n\n");
}

int main()
{
#define SIZE 3
	struct cblist list;
	struct entry mem[1 << SIZE];

#define DBG(x) do {puts(#x); x; cbl_print(&list);} while (0)
	DBG(cbl_init(&list, mem, sizeof(mem)));
	DBG(cbl_set(&list, hash("a"), 1));
	DBG(cbl_set(&list, hash("b"), 2));
	DBG(cbl_set(&list, hash("c"), 3));
	DBG(cbl_set(&list, hash("d"), 4));
	DBG(cbl_set(&list, hash("d"), 5));
	DBG(printf("b = %ld\n", cbl_get(&list, hash("b"))));
	DBG(printf("e = %ld\n", cbl_get(&list, hash("e"))));
	DBG(cbl_set(&list, hash("d"), 4));
	DBG(cbl_set(&list, hash("e"), 5));
	DBG(cbl_set(&list, hash("f"), 6));
	DBG(cbl_set(&list, hash("g"), 7));
	DBG(cbl_set(&list, hash("h"), 8));
	DBG(printf("f = %ld\n", cbl_get(&list, hash("f"))));
	DBG(printf("g = %ld\n", cbl_get(&list, hash("g"))));
	DBG(printf("h = %ld\n", cbl_get(&list, hash("h"))));
	DBG(printf("i = %ld\n", cbl_get(&list, hash("i"))));

	return 0;
}
