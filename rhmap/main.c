#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RHMAP_VAL int
#include "rhmap.h"

void map_print(struct map *m)
{
	for (unsigned i = 0; i < m->size; i++) {
		printf("%d: ", i);
		printf("[Key: %llu, ", m->buckets[i].key);
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

struct bucket map_mem[32];
int main()
{
	struct map m;
	map_init(&m, map_mem, sizeof(map_mem));
#define INS(x,y) map_insert(&m, map_hash(x, sizeof(x)-1), y)
	INS("Alfa", 1);
	INS("Bravo", 2);
	INS("Charlie", 3);
	INS("Delta", 4);
	INS("Echo", 5);
	INS("Foxtrot", 6);
	INS("Golf", 7);
	INS("Hotel", 8);
	INS("Indigo", 9);
	INS("Julia", 10);
	INS("Kilo", 11);
	INS("Lima", 12);
	INS("Mike", 13);
	INS("November", 14);
	INS("Oscar", 15);
	INS("Papa", 16);
	INS("Quebec", 17);
	INS("Romeo", 18);
	INS("Sierra", 19);
	INS("Tango", 20);
	INS("Uniform", 21);
	INS("Victor", 22);
	INS("Whiskey", 23);
	INS("X-ray", 24);
	INS("Yankee", 25);
	INS("Zulu", 26);

	map_print(&m);

	test_search(&m, "Alfa");
	test_search(&m, "Bravo");
	test_search(&m, "Charlie");
	test_search(&m, "Delta");
	test_search(&m, "Echo");
	test_search(&m, "Foxtrot");
	test_search(&m, "Golf");
	test_search(&m, "Hotel");
	test_search(&m, "Indigo");
	test_search(&m, "Julia");
	test_search(&m, "Kilo");
	test_search(&m, "Lima");
	test_search(&m, "Mike");
	test_search(&m, "November");
	test_search(&m, "Oscar");
	test_search(&m, "Papa");
	test_search(&m, "Quebec");
	test_search(&m, "Romeo");
	test_search(&m, "Sierra");
	test_search(&m, "Tango");
	test_search(&m, "Uniform");
	test_search(&m, "Victor");
	test_search(&m, "Whiskey");
	test_search(&m, "X-ray");
	test_search(&m, "Yankee");
	test_search(&m, "Zulu");

	return 0;
}
