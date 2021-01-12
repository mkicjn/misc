#include "rhmap.h"

unsigned long map_hash(unsigned char *mem, unsigned len)
{
	unsigned long h = 5381;
	for (int i = 0; i < len; i++)
		h = mem[i] + (h << 5) + h;
	return h > UNUSED ? h : UNUSED+1;
	// ^ Don't return reserved keys
}

static struct bucket *map_index(struct map *m, unsigned long key)
{
	int i = key % m->size;
	while (m->buckets[i].key != UNUSED) {
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
	for (int i = 0; i < m->size; i++)
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

val_t *map_lookup(struct map *m, unsigned long key)
{
	struct bucket *b = map_index(m, key);
	if (b)
		return &b->val;
	return NULL;
}
