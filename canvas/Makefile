CC = clang
CFLAGS = -O
LD = ld
LDFLAGS = -lm -lSDL2

BACKEND = sdl2

a.out: main.o canvas.o
	$(CC) $(LDFLAGS) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $< -c -o $@

canvas.o: canvas_$(BACKEND).c canvas.h
	$(CC) $(CFLAGS) $< -c -o $@

.PHONY: clean
clean:
	rm -f a.out *.o
cleaner: clean
	rm -f .*~ *~
