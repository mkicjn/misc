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
	$(CC) $(LDFLAGS) $^ -o $@

# Object files

canvas.o: ../src/canvas_$(BACKEND).c ../src/canvas.h
	$(CC) $(CFLAGS) $< -c -o $@

cursor.o: ../src/cursor.c ../src/cursor.h
	$(CC) $(CFLAGS) $< -c -o $@

line%.o: src/line%.c src/line%.h
	$(CC) $(CFLAGS) $< -c -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

# Cleanup

.PHONY: clean
clean:
	rm -f $(ALL) *.o
