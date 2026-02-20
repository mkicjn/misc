//`which tcc` $CFLAGS -run $0 ""$@""; exit $?
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

size_t row_len;
int rowcmp(const void *row1_ptr, const void *row2_ptr)
{
	// Sort two entries in a "rows" array based on the contents they point to
	const char *row1 = *(const char **)row1_ptr;
	const char *row2 = *(const char **)row2_ptr;
	return memcmp(row1, row2, row_len);
}

char **rows;
char *rowbuf;
void bwt(const char *data, size_t data_len)
{
	// Allocates rowbuf as char[data_len * 2], rows as char (*[data_len])[data_len]
	size_t rowbuf_len = data_len * 2;
	char *rowbuf = NULL;

	// Concatenate input data to itself so that each row can be a simple offset
	rowbuf = malloc(rowbuf_len + 1);
	memcpy(&rowbuf[0],        data, data_len);
	memcpy(&rowbuf[data_len], data, data_len);
	rowbuf[rowbuf_len] = '\0';

	// Put the rows in an array and sort them lexicographically
	size_t n_rows = data_len;
	rows = malloc(n_rows * sizeof(*rows));
	for (int i = 0; i < n_rows; i++)
		rows[i] = &rowbuf[i];

	row_len = data_len;
	qsort(rows, n_rows, sizeof(*rows), rowcmp);
}

int main()
{
	// Read all stdin
	size_t buf_len = 0;
	size_t buf_cap = 4096;
	char *buf = malloc(buf_cap);
	while (!feof(stdin)) {
		if (buf_len == buf_cap) {
			buf_cap *= 2;
			buf = realloc(buf, buf_cap);
		}
		ssize_t n_read = fread(&buf[buf_len], 1, buf_cap - buf_len, stdin);
		if (n_read <= 0)
			break;
		buf_len += n_read;
	}

	// Perform Burrows-Wheeler on it
	bwt(buf, buf_len);
	for (int i = 0; i < buf_len; i++)
		putchar(rows[i][buf_len - 1]);

	free(rows);
	free(rowbuf);
	return 0;
}
