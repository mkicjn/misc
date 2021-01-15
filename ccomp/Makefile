CC = clang
CFLAGS = -Wall -Wextra -pedantic -g
LD = $(CC)
LDFLAGS = -lm

ALL = lextest

all: $(ALL)

lextest: lextest.o lexer.o token.o
	$(LD) $(LDFLAGS) $^ -o $@

lextest.o: lextest.c
	$(CC) $(CFLAGS) -c $< -o $@

lexer.o: src/lexer.c src/token.h
	$(CC) $(CFLAGS) -c $< -o $@

token.o: src/token.c src/token.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o $(ALL)
