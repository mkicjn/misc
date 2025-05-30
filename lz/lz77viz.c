//$(which tcc) $CFLAGS -run $0 $@; exit $?
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static char obuf[256] = {0};
static char ibuf[256] = {0};
static int obuf_len = 0;
static int ibuf_len = 0;

void get_longest_match(int *ret_pos, int *ret_len)
{
	int max_pos = 0;
	int max_len = 0;
	for (int pos = 0; pos < obuf_len - max_len; pos++) {
		int len;
		for (len = 0; len < ibuf_len - 1 && pos + len < obuf_len; len++) {
			if (obuf[pos + len] != ibuf[len])
				break;
		}
		if (len > max_len) {
			max_pos = pos;
			max_len = len;
		}
	}
	*ret_pos = max_pos;
	*ret_len = max_len;
}

void transfer(int n)
{
	int excess = obuf_len + n - sizeof(obuf);
	if (excess > 0) {
		memmove(&obuf[0], &obuf[excess], obuf_len - excess);
		obuf_len -= excess;
	}

	memcpy(&obuf[obuf_len], ibuf, n);
	obuf_len += n;

	memmove(&ibuf[0], &ibuf[n], ibuf_len - n);
	ibuf_len -= n;
}

void refill(void)
{
	while (!feof(stdin) && ibuf_len < sizeof(ibuf)) {
		int c = getchar();
		if (c == EOF)
			break;
		ibuf[ibuf_len++] = c;
	}
}

void encode(void)
{
	do {
		refill();

		int pos, len, next;
		get_longest_match(&pos, &len);
		next = ibuf[len];

		printf("\033[90m%.*s\033[m%c", len, &obuf[pos], next);
		//putchar(pos);
		//putchar(len);
		//putchar(next);
		transfer(len+1);
	} while (ibuf_len > 0);
}

void decode(void)
{
	while (!feof(stdin)) {
		int pos = getchar();
		int len = getchar();
		int next = getchar();
		if (pos == EOF || len == EOF || next == EOF)
			break;

		printf("\033[90m%.*s\033[m%c", len, &obuf[pos], next);
		memcpy(ibuf, &obuf[pos], len);
		ibuf[len] = next;
		ibuf_len = len + 1;

		//printf("%.*s", len+1, ibuf);
		transfer(len+1);
	}
}

int main(int argc, char **argv)
{
	(void)argv;
	if (argc > 1)
		decode();
	else
		encode();
	return 0;
}
