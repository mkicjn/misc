//$(which tcc) $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

size_t row_len;
int rowcmp(const void *row1_ptr, const void *row2_ptr)
{
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

	rowbuf = malloc(rowbuf_len + 1);
	memcpy(&rowbuf[0],        data, data_len);
	memcpy(&rowbuf[data_len], data, data_len);
	rowbuf[rowbuf_len] = '\0';

	size_t n_rows = data_len;
	rows = malloc(n_rows * sizeof(*rows));
	for (int i = 0; i < n_rows; i++)
		rows[i] = &rowbuf[i];

	row_len = data_len;
	qsort(rows, n_rows, sizeof(*rows), rowcmp);
}

int main(int argc, char **argv)
{
	if (argc < 2)
		return 1;

	const char *data = argv[1];
	size_t data_len = strlen(argv[1]);
	bwt(data, data_len);

	for (int i = 0; i < data_len; i++)
		putchar(rows[i][data_len - 1]);
	putchar('\n');

	free(rows);
	free(rowbuf);
	return 0;
}
