CC = clang
CFLAGS = -O
LD = ld
LDFLAGS = -lm -lSDL2

BACKEND = sdl2

all: paint test

paint:	paint.o canvas.o
	$(CC) $(LDFLAGS) $^ -o $@

test:	test.o canvas.o
	$(CC) $(LDFLAGS) $^ -o $@

test.o: test.c
	$(CC) $(CFLAGS) $< -c -o $@

paint.o: paint.c
	$(CC) $(CFLAGS) $< -c -o $@

canvas.o: src/canvas_$(BACKEND).c src/canvas.h
	$(CC) $(CFLAGS) $< -c -o $@

.PHONY: clean
clean:
	rm -f paint test *.o
cleaner: clean
	rm -f rm .*~ *~ */.*~ */*~
