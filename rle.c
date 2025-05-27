//$(which tcc) $CFLAGS -run $0 $@; exit $?
#include <stdio.h>
#include <limits.h>

void rle_emit(int count, int last, FILE *out)
{
	if (count <= 0)
		return;
	if (last == 0) {
		if (count == 1) {
			// Special case: use \0 to escape a single \0
			fputc(0, out);
			fputc(0, out);
		} else {
			// Otherwise use RLE as usual
			goto rle;
		}
	} else if (last != 0 && count < 4) {
		for (int i = 0; i < count; i++)
			fputc(last, out);
	} else {
rle:		fputc(0, out);
		fputc(count, out);
		fputc(last, out);
	}
}

void rle_encode(FILE *in, FILE *out)
{
	int last = 0;
	int count = 0;
	while (!feof(in)) {
		int c = fgetc(in);
		if (c == EOF)
			break;
		if (c != last || count == 0xff) {
			// If change in pattern or out of range, do RLE
			rle_emit(count, last, out);
			last = c;
			count = 1;
		} else {
			// If no change in run, keep counting
			count++;
		}
	}
	// Do RLE again to capture the last run
	rle_emit(count, last, out);
}

void rle_decode(FILE *in, FILE *out)
{
	while (!feof(in)) {
		int c = fgetc(in);
		int count = 1;
		if (c == EOF)
			break;
		// If we receive a \0, it is escaping something
		if (c == 0) {
			// Next byte is run length, if not another \0
			c = fgetc(in);
			if (c != 0) {
				count = c;
				c = fgetc(in);
			}
		}
		// Output the run
		for (int i = 0; i < count; i++)
			fputc(c, out);
	}
}

int main(int argc, char **argv)
{
	(void)argv;

	// Compress/decompress based on option
	if (argc > 1)
		rle_decode(stdin, stdout);
	else
		rle_encode(stdin, stdout);

	return 0;
}
