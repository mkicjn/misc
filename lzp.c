//usr/bin/env tcc $CFLAGS -run $0 $@; exit $?
#include <stdio.h>
#include <stdint.h>

uint8_t in[8];
size_t in_len;
uint8_t pred[1 << 16] = {0};
uint16_t hash = 0;

void encode(void)
{
	for (;;) {
		// Get up to 8 bytes
		in_len = fread(in, 1, 8, stdin);

		// For each byte:
		uint8_t mask = 0;
		for (int i = 0; i < in_len; i++) {
			int c = in[i];
			// Put a 1 in the mask if the predictor hit
			if (pred[hash] == c)
				mask |= (1 << i);
			// Update the predictor and hash
			pred[hash] = c;
			hash = (hash << 8) | c;
		}

		// Output the mask and any missed bytes
		putchar(mask);
		for (int i = 0; i < in_len; i++)
			if (~mask & (1 << i))
				putchar(in[i]);
		
		// Quit if we ran out of bytes
		if (in_len < 8)
			return;
	}
}

void decode(void)
{
	for (;;) {
		// Expect a mask byte
		int mask = getchar();
		if (mask == EOF)
			return;

		// For each byte predicted by the mask:
		for (int i = 0; i < 8; i++) {
			int c = pred[hash];
			// If the mask says we're going to miss, expect a correction
			if (~mask & (1 << i)) {
				c = getchar();
				if (c == EOF)
					return;
			}
			// Output the prediction
			putchar(c);
			// Update the predictor and hash
			pred[hash] = c;
			hash = (hash << 8) | c;
		}
	};
}

int main(int argc, char **argv)
{
	(void)argv;

	// Compress/decompress based on option
	if (argc > 1)
		decode();
	else
		encode();

	return 0;
}
