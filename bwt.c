//$(which tcc) $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

size_t row_len;
size_t n_rows;
char **rows;
int rowcmp(const void *row1_ptr, const void *row2_ptr)
{
	const char *row1 = *(const char **)row1_ptr;
	const char *row2 = *(const char **)row2_ptr;
	return memcmp(row1, row2, row_len);
}

int main(int argc, char **argv)
{
	if (argc < 2)
		return 1;

	size_t arg_len = strlen(argv[1]);
	size_t data_len = arg_len * 2;
	char *data = NULL;

	data = malloc(data_len + 1);
	memcpy(&data[0],       argv[1], arg_len);
	memcpy(&data[arg_len], argv[1], arg_len);
	data[data_len] = '\0';

	row_len = arg_len;
	n_rows = arg_len;
	rows = malloc(n_rows * sizeof(*rows));
	for (int i = 0; i < n_rows; i++)
		rows[i] = &data[i];

	qsort(rows, n_rows, sizeof(*rows), rowcmp);

	for (int i = 0; i < n_rows; i++)
		putchar(rows[i][arg_len - 1]);
	putchar('\n');

	free(rows);
	free(data);
	return 0;
}
