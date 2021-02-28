#include "rhmap.h"

static struct bucket *map_index(struct map *m, unsigned long long key)
{
	size_t i = key % m->size;
	size_t c = m->max_dist;
	while (c-- >= 0 && m->buckets[i].key != UNUSED) {
		if (m->buckets[i].key == key)
			return &m->buckets[i];
		i = (i+1) % m->size;
	}
	return NULL;
}

void map_init(struct map *m, void *b, size_t len)
{
	m->buckets = b;
	m->size = len / sizeof(struct bucket);
	m->pop = 0;
	m->max_dist = 0;
	for (size_t i = 0; i < m->size; i++)
		m->buckets[i].key = UNUSED;
}

bool map_insert(struct map *m, unsigned long long key, RHMAP_VAL val)
{
	size_t i = key % m->size;
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

bool map_remove(struct map *m, unsigned long long key)
{
	struct bucket *b = map_index(m, key);
	if (b) {
		b->key = TOMBSTONE;
		m->pop--;
		return true;
	}
	return false;
}

RHMAP_VAL *map_search(struct map *m, unsigned long long key)
{
	struct bucket *b = map_index(m, key);
	if (b)
		return &b->val;
	return NULL;
}
