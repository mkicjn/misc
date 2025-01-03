#include <stdio.h>
#include <stdint.h>

uint8_t in[8];
size_t in_len;
uint8_t pred[1 << 16] = {0};
uint16_t hash = 0;

void encode(void)
{
	for (;;) {
		// Get up to 8 characters
		in_len = fread(in, 1, 8, stdin);

		// For each character:
		uint8_t hits = 0;
		for (int i = 0; i < in_len; i++) {
			// Put a 1 in the mask if the predictor hit
			if (pred[hash] == in[i])
				hits |= (1 << i);
			// Update the predictor and hash
			pred[hash] = in[i];
			hash = (hash << 8) | in[i];
		}

		// Output the mask and any missed bytes
		putchar(hits);
		for (int i = 0; i < in_len; i++)
			if (~hits & (1 << i))
				putchar(in[i]);
		
		// Quit if we ran out of characters part way
		if (in_len < 8)
			return;
	}
}

void decode(void)
{
	for (;;) {
		// Expect a mask character
		int hits = getchar();
		if (hits == EOF)
			return;

		// For each bit in the mask:
		for (int i = 0; i < 8; i++) {
			// If the mask says we're going to miss, expect a correction
			if (~hits & (1 << i)) {
				int c = getchar();
				if (c == EOF)
					return;
				pred[hash] = c;
			}
			// Output the prediction and update hash
			putchar(pred[hash]);
			hash = (hash << 8) | pred[hash];
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
