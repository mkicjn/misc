#include "rhmap.h"

unsigned long map_hash(const char *str, unsigned n)
{
	unsigned long k = 5381;
	while (n --> 0)
		k = (k << 5) + k + *str++;
	return k > UNUSED ? k : UNUSED + 1; // Avoid reserved keys
}

static struct bucket *map_index(struct map *m, unsigned long key)
{
	int i = key % m->size;
	int c = m->max_dist;
	while (c-- >= 0 && m->buckets[i].key != UNUSED) {
		if (m->buckets[i].key == key)
			return &m->buckets[i];
		i = (i+1) % m->size;
	}
	return NULL;
}

void map_init(struct map *m, void *b, unsigned len)
{
	m->buckets = b;
	m->size = len / sizeof(struct bucket);
	m->pop = 0;
	m->max_dist = 0;
	for (unsigned i = 0; i < m->size; i++)
		m->buckets[i].key = UNUSED;
}

bool map_insert(struct map *m, unsigned long key, val_t val)
{
	unsigned i = key % m->size;
	struct bucket ins;
	if (m->pop+1 > m->size)
		return false;
	ins.key = key;
	ins.val = val;
	ins.dist = 0;
	while (m->buckets[i].key != UNUSED) {
		if (m->buckets[i].dist < ins.dist) {
			struct bucket tmp;
			if (m->buckets[i].key == TOMBSTONE)
				break;
			if (ins.dist+1 > m->max_dist)
				m->max_dist = ins.dist+1;
			tmp = ins;
			ins = m->buckets[i];
			m->buckets[i] = tmp;
		}
		i = (i+1) % m->size;
		ins.dist++;
	}
	m->buckets[i] = ins;
	m->pop++;
	return true;
}

bool map_remove(struct map *m, unsigned long key)
{
	struct bucket *b = map_index(m, key);
	if (b) {
		b->key = TOMBSTONE;
		m->pop--;
		return true;
	}
	return false;
}

val_t *map_search(struct map *m, unsigned long key)
{
	struct bucket *b = map_index(m, key);
	if (b)
		return &b->val;
	return NULL;
}
