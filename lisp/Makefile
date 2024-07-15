CFLAGS=-Os -Wall -Wextra -pedantic

ALL=a.out

a.out: lisp.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(ALL)
