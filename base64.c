//$(which tcc) $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>


// Streaming base64 encoding

static const char base64_str[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static unsigned buf = 0;
static int buf_len = 0;

bool base64_push(int c)
{
	if (buf_len + 8 > sizeof(buf) * CHAR_BIT)
		return false;
	buf = (buf << 8) | (c & 0xff);
	buf_len += 8;
	return true;
}

bool base64_pull(int *c)
{
	if (buf_len < 6)
		return false;
	buf_len -= 6;
	*c = base64_str[(buf >> buf_len) & 0x3f];
	return true;
}

bool base64_pad(int *c)
{
	if (buf_len >= 6) {
		return base64_pull(c);
	} else if (buf_len > 0) {
		buf_len -= 6;
		*c = base64_str[(buf << -buf_len) & 0x3f];
		return true;
	} else if (buf_len < 0) {
		*c = '=';
		buf_len += 2;
		return true;
	}
	return false;
}

// Streaming base64 decoding

static char base64d_str[256] = {0};

void base64d_str_init(void)
{
	for (int i = 0; i < 256; i++)
		base64d_str[i] = -1;
	for (int i = 0; i < 64; i++)
		base64d_str[base64_str[i]] = i;
}

bool base64d_push(int c)
{
	c = base64d_str[c & 0xff];
	if (c < 0 || buf_len + 6 > sizeof(buf) * CHAR_BIT)
		return false;
	buf = (buf << 6) | (c & 0x3f);
	buf_len += 6;
	return true;
}

bool base64d_pull(int *c)
{
	if (buf_len < 8)
		return false;
	buf_len -= 8;
	*c = (buf >> buf_len) & 0xff;
	return true;
}


// Main

void putwrap(int c)
{
	static const int lim = 76;
	static int chars_out = 0;
	putchar(c);
	if (++chars_out >= lim) {
		putchar('\n');
		chars_out -= lim;
	}
}

int main(int argc, char **argv)
{
	int c;
	if (argc > 1) {
		base64d_str_init();
		for (c = getchar(); c != EOF; c = getchar()) {
			base64d_push(c);
			while (base64d_pull(&c))
				putchar(c);
		}
	} else {
		for (c = getchar(); c != EOF; c = getchar()) {
			base64_push(c);
			while (base64_pull(&c))
				putwrap(c);
		}
		while (base64_pad(&c))
			putwrap(c);
	}
	putchar('\n');
	return 0;
}
