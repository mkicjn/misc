#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rhmap.h"

void map_print(struct map *m)
{
	for (unsigned i = 0; i < m->size; i++) {
		printf("%d: ", i);
		printf("[Key: %lu, ", m->buckets[i].key);
		printf("Val: %d, ", m->buckets[i].val);
		printf("Dist: %ld]\n", m->buckets[i].dist);
	}
}

void test_search(struct map *m, const char *s)
{
	printf("Lookup %s: ", s);
	int *res = map_search(m, map_hash(s, strlen(s)));
	if (res != NULL)
		printf("%d\n", *res);
	else
		printf("FAILED!\n");
}

struct bucket map_mem[9];
int main()
{
	struct map m;
	map_init(&m, map_mem, sizeof(map_mem));
#define INS(x,y) map_insert(&m, map_hash(x, sizeof(x)-1), y)
	INS("Adam", 12345);
	INS("Bobby", 2787);
	INS("Claire", 329);
	INS("Doug", 49297);
	INS("Eliza", 5175);
	INS("Fred", 67439);
	INS("Greg", 79256);
	INS("Henry", 8468);
	INS("Isaac", 9231);

	map_print(&m);

	test_search(&m, "Adam");
	test_search(&m, "Bobby");
	test_search(&m, "Claire");
	test_search(&m, "Doug");
	test_search(&m, "Eliza");
	test_search(&m, "Fred");
	test_search(&m, "Greg");
	test_search(&m, "Henry");
	test_search(&m, "Isaac");
	return 0;
}
