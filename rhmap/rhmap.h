#ifndef RHMAP_H
#define RHMAP_H
#include <stddef.h>
#include <stdbool.h>

#ifndef RHMAP_VAL
#define RHMAP_VAL void *
#endif

struct bucket {
	unsigned long long key;
	size_t dist;
	RHMAP_VAL val;
};

struct map {
	struct bucket *buckets;
	size_t size;
	size_t pop;
	size_t max_dist;
	RHMAP_VAL result;
};

enum reserved_key {
	TOMBSTONE,
	UNUSED
};

unsigned long long map_hash(const char *mem, size_t len);
void map_init(struct map *m, void *b, size_t len);
bool map_insert(struct map *m, unsigned long long key, RHMAP_VAL val);
bool map_remove(struct map *m, unsigned long long key);
RHMAP_VAL *map_search(struct map *m, unsigned long long key);

#endif
