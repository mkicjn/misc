CC=clang
CFLAGS=-pedantic -Wall -Wextra -std=gnu99

test:
	$(CC) $(CFLAGS) *.c
