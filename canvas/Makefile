CC = clang
CFLAGS = -O
LD = ld
LDFLAGS = -lm -lSDL2

BACKEND = sdl2

TARGETS = test draw life

all: $(TARGETS)

life:	life.o canvas.o
	$(CC) $(LDFLAGS) $^ -o $@

draw:	draw.o canvas.o
	$(CC) $(LDFLAGS) $^ -o $@

test:	test.o canvas.o
	$(CC) $(LDFLAGS) $^ -o $@


canvas.o: src/canvas_$(BACKEND).c src/canvas.h
	$(CC) $(CFLAGS) $< -c -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

.PHONY: clean
clean:
	rm -f $(TARGETS) *.o
cleaner: clean
	rm -f rm .*~ *~ */.*~ */*~
