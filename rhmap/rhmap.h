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
	val_t result;
};

enum reserved_key {
	TOMBSTONE,
	UNUSED
};

#endif
