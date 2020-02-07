CC = clang
CFLAGS = -Os
SRCS =

a.out: main.c
	$(CC) $(CFLAGS) $< -o $@

code.c: names.c
	find $(SRCS) | grep ".c$$" | sed 's/.*/#include "&"/' > $@

.PHONY: clean
clean:
	rm -f a.out .*~ *~
