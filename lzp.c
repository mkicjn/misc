#include <stdio.h>
#include <limits.h>

#ifndef TABLE_SIZE
#define TABLE_SIZE (1 << 16)
#endif

void lzp_encode(FILE *in, FILE *out)
{
	char pred_table[TABLE_SIZE] = {0};
	size_t hash = 0;
	while (!feof(in)) {
		char mask = 0;
		char missed[CHAR_BIT];
		size_t num_missed = 0;
		// For each bit in the char mask:
		for (int i = 0; i < CHAR_BIT; i++) {
			// Get next character
			int c = fgetc(in);
			if (c == EOF)
				break;
			// Check if correctly predicted by hash
			if (pred_table[hash % sizeof(pred_table)] == c) {
				// If so, indicate on the mask
				mask |= 1 << i;
			} else {
				// Otherwise, fix the table and insert the character into the buffer
				pred_table[hash % sizeof(pred_table)] = c;
				missed[num_missed++] = c;
			}
			// Update the hash with the input character
			hash = (hash << 4) ^ c;
		}
		// Output the char mask of correctly-predicted characters
		fputc(mask, out);
		// Output incorrectly-predicted characters in the buffer
		for (int i = 0; i < num_missed; i++)
			fputc(missed[i], out);
	}
}

void lzp_decode(FILE *in, FILE *out)
{
	char pred_table[TABLE_SIZE] = {0};
	size_t hash = 0;
	while (!feof(in)) {
		// Expect a mask character
		int mask = fgetc(in);
		if (mask == EOF)
			break;
		// For each bit in the mask:
		for (int i = 0; i < CHAR_BIT; i++) {
			int c;
			// Check if the character is correctly predicted by the table
			if (mask & (1 << i)) {
				// If so, output the prediction
				c = pred_table[hash % sizeof(pred_table)];
				fputc(c, out);
			} else {
				// Otherwise, expect the correct character, fix the table, and output it
				c = fgetc(in);
				if (c == EOF)
					break;
				pred_table[hash % sizeof(pred_table)] = c;
				fputc(c, out);
			}
			// Update the hash with the output character
			hash = (hash << 4) ^ c;
		}
	}
}

int main(int argc, char **argv)
{
	// Parse command line option
	char opt = argc == 2 ? argv[1][1] : 0;

	// Show usage if invalid
	if (opt != 'c' && opt != 'd') {
		printf("Usage: %s <-c|-d>\n", argc > 0 ? argv[0] : "./lzp");
		return 1;
	}

	// Compress/decompress based on option
	if (opt == 'c')
		lzp_encode(stdin, stdout);
	else
		lzp_decode(stdin, stdout);
	return 0;
}
