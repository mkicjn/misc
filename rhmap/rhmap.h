#ifndef RHMAP_H
#define RHMAP_H
#include <stdbool.h>
#include <stddef.h>

typedef int val_t;

struct bucket {
	unsigned long key;
	long dist;
	val_t val;
};

struct map {
	struct bucket *buckets;
	unsigned size;
	unsigned pop;
	unsigned max_dist;
	val_t result;
};

enum reserved_key {
	TOMBSTONE,
	UNUSED
};

unsigned long map_hash(const char *mem, unsigned len);
void map_init(struct map *m, void *b, unsigned len);
bool map_insert(struct map *m, unsigned long key, val_t val);
bool map_remove(struct map *m, unsigned long key);
val_t *map_search(struct map *m, unsigned long key);

#endif
