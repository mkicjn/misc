CC = clang
CFLAGS = -Os -g
SRCS = code/*.c

a.out: main.c *.h
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f a.out .*~ *~
