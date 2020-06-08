# Parameters

CC = clang
CFLAGS = -Os -Wall
LD = ld
LDFLAGS = -lm -lSDL2

BACKEND = sdl2

ALL = bench

# Executables

all: $(ALL)

bench:	bench.o canvas.o line1.o line2.o
	$(CC) $^ -o $@ $(LDFLAGS)

# Object files

canvas.o: ../src/canvas_$(BACKEND).c ../src/canvas.h
	$(CC) $< -c -o $@ $(CFLAGS)

cursor.o: ../src/cursor.c ../src/cursor.h
	$(CC) $< -c -o $@ $(CFLAGS)

line%.o: src/line%.c src/line%.h
	$(CC) $< -c -o $@ $(CFLAGS)

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

# Cleanup

.PHONY: clean
clean:
	rm -f $(ALL) *.o
