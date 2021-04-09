#ifndef RHMAP_H
#define RHMAP_H
#include <stddef.h>
#include <stdbool.h>

#ifndef RHMAP_KEY
#define RHMAP_KEY unsigned long long
#endif

enum {TOMBSTONE, UNUSED};

#define DECLARE_RHMAP(map, type)					\
struct map {								\
	struct map##_bucket {						\
		RHMAP_KEY key;						\
		size_t dist;						\
		type val;						\
	} *buckets;							\
	size_t size;							\
	size_t pop;							\
	size_t max_dist;						\
};									\
									\
void map##_init(struct map *m, void *b, size_t len);			\
bool map##_insert(struct map *m, RHMAP_KEY key, type val);		\
bool map##_remove(struct map *m, RHMAP_KEY key);			\
type *map##_search(struct map *m, RHMAP_KEY key);			\
									\
static struct map##_bucket *map##_index(struct map *m, RHMAP_KEY key)	\
{									\
	size_t i = key % m->size;					\
	size_t c = m->max_dist;						\
	while (c-- >= 0 && m->buckets[i].key != UNUSED) {		\
		if (m->buckets[i].key == key)				\
			return &m->buckets[i];				\
		i = (i+1) % m->size;					\
	}								\
	return NULL;							\
}									\
									\
void map##_init(struct map *m, void *b, size_t len)			\
{									\
	m->buckets = b;							\
	m->size = len / sizeof(struct map##_bucket);			\
	m->pop = 0;							\
	m->max_dist = 0;						\
	for (size_t i = 0; i < m->size; i++)				\
		m->buckets[i].key = UNUSED;				\
}									\
									\
bool map##_insert(struct map *m, RHMAP_KEY key, type val)		\
{									\
	size_t i = key % m->size;					\
	struct map##_bucket ins;					\
	if (m->pop+1 > m->size)						\
		return false;						\
	ins.key = key;							\
	ins.val = val;							\
	ins.dist = 0;							\
	while (m->buckets[i].key != UNUSED) {				\
		if (m->buckets[i].dist < ins.dist) {			\
			struct map##_bucket tmp;			\
			if (m->buckets[i].key == TOMBSTONE)		\
				break;					\
			if (ins.dist+1 > m->max_dist)			\
				m->max_dist = ins.dist+1;		\
			tmp = ins;					\
			ins = m->buckets[i];				\
			m->buckets[i] = tmp;				\
		}							\
		i = (i+1) % m->size;					\
		ins.dist++;						\
	}								\
	m->buckets[i] = ins;						\
	m->pop++;							\
	return true;							\
}									\
									\
bool map##_remove(struct map *m, RHMAP_KEY key)				\
{									\
	struct map##_bucket *b = map##_index(m, key);			\
	if (b) {							\
		b->key = TOMBSTONE;					\
		m->pop--;						\
		return true;						\
	}								\
	return false;							\
}									\
									\
type *map##_search(struct map *m, RHMAP_KEY key)			\
{									\
	struct map##_bucket *b = map##_index(m, key);			\
	if (b)								\
		return &b->val;						\
	return NULL;							\
}

#endif
