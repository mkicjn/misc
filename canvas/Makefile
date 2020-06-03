# Parameters

CC = clang
CFLAGS = -O -Wall
LD = ld
LDFLAGS = -lm -lSDL2

BACKEND = sdl2

ALL = test draw life

# Executables

all: $(ALL)

life:	life.o canvas.o
	$(CC) $(LDFLAGS) $^ -o $@

draw:	draw.o canvas.o line.o
	$(CC) $(LDFLAGS) $^ -o $@

test:	test.o canvas.o
	$(CC) $(LDFLAGS) $^ -o $@

# Object files

canvas.o: src/canvas_$(BACKEND).c src/canvas.h
	$(CC) $(CFLAGS) $< -c -o $@

line.o: src/line.c src/line.h
	$(CC) $(CFLAGS) $< -c -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

# Cleanup

.PHONY: clean
clean:
	rm -f $(ALL) *.o
cleaner: clean
	rm -f rm .*~ *~ */.*~ */*~
