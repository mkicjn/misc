//$(which tcc) $CFLAGS -run $0 $@; exit $?
#include <stdio.h>
#include <stdint.h>

static char pred[256][256] = {0};
static int hist[2] = {0};

int predict(void)
{
	// Query the prediction table
	char *bucket = &pred[hist[0]][hist[1]];
	return *bucket;
}

void confirm(int c)
{
	// Update the prediction table and history
	char *bucket = &pred[hist[0]][hist[1]];
	*bucket = c;
	hist[1] = hist[0];
	hist[0] = c;
}

void encode(void)
{
	while (!feof(stdin)) {
		int mask = 0;
		int miss[8], n_miss = 0;
		for (int i = 0; i < 8; i++) {
			int c = getchar();
			if (c == EOF)
				break;
			// Are we going to predict correctly later?
			if (predict() == c)
				mask |= (1 << i); // Yes: set mask bit
			else
				miss[n_miss++] = c; // No: output correction
			confirm(c);
		}
		putchar(mask);
		for (int i = 0; i < n_miss; i++)
			putchar(miss[i]);
	}
}

void decode(void)
{
	while (!feof(stdin)) {
		int mask = getchar();
		if (mask == EOF)
			break;
		for (int i = 0; i < 8; i++) {
			int c;
			// Does the mask say we will predict correctly?
			if (mask & (1 << i)) {
				c = predict(); // Yes: use the prediction
			} else {
				c = getchar(); // No: expect a correction
				if (c == EOF)
					break;
			}
			confirm(c);
			putchar(c);
		}
	}
}

int main(int argc, char **argv)
{
	(void)argv;
	// Compress/decompress based on argument
	if (argc > 1)
		decode();
	else
		encode();
	return 0;
}
