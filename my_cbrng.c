#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint32_t cbrng(int n)
{ // Inspired by the middle square Weyl sequence and Squares
	static uint64_t const s = 0xB5AD4ECEDA1CE2A9UL;
	
	uint64_t x = n * s;
	x *= x ^ s;
	return x >> 32;
}

void random_pbm(int width, int height, int i)
{
	uint32_t buf = cbrng(i);
	int b = 32;
	printf("P1\n%d %d\n", width, height);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			printf("%d ", buf & 1);
			buf >>= 1;
			if (--b == 0) {
				buf = cbrng(++i);
				b = 32;
			}
		}
		printf("\n");
	}
}

int main(int argc, char **argv)
{
	enum {RAW = 1, CSV, PBM} fmt = 0;

	if (argc < 2) {
		printf("No selection\n");
		return 1;
	}
	if (!strcasecmp(argv[1], "raw"))
		fmt = RAW;
	if (!strcasecmp(argv[1], "csv"))
		fmt = CSV;
	if (!strcasecmp(argv[1], "pbm"))
		fmt = PBM;

	int x, y, n = 0;
	switch (fmt) {
	case RAW:
		if (argc < 3) {
			printf("Not enough arguments\n");
			return 1;
		}
		sscanf(argv[2], "%d", &y);
		if (argc > 3)
			sscanf(argv[3], "%d", &n);
		for (int i = 0; i < y; i++) {
			unsigned u = cbrng(++n);
			fwrite(&u, sizeof(unsigned), 1, stdout);
		}
		break;
	case CSV:
		if (argc < 4) {
			printf("Not enough arguments\n");
			return 1;
		}
		sscanf(argv[2], "%d", &x);
		sscanf(argv[3], "%d", &y);
		if (argc > 4)
			sscanf(argv[4], "%d", &n);
		for (int i = 0; i < y; i++) {
			for (int j = 0; j < x; j++)
				printf("%d,", cbrng(++n));
			putchar('\n');
		}
		break;
	case PBM:
		if (argc < 4) {
			printf("Not enough arguments\n");
			return 1;
		}
		sscanf(argv[2], "%d", &x);
		sscanf(argv[3], "%d", &y);
		if (argc > 4)
			sscanf(argv[4], "%d", &n);
		random_pbm(x, y, n);
		break;
	default:
		printf("Invalid selection\n");
		return 1;
	}
	return 0;
}
