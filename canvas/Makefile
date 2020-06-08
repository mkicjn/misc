# Parameters

CC = clang
CFLAGS = -Os -Wall
LD = ld
LDFLAGS = -lm -lSDL2

BACKEND = sdl2

ALL = grad draw life dot

# Executables

all: $(ALL)

life:	life.o canvas.o
	$(CC) $^ -o $@ $(LDFLAGS)

draw:	draw.o canvas.o cursor.o
	$(CC) $^ -o $@ $(LDFLAGS)

grad:	grad.o canvas.o
	$(CC) $^ -o $@ $(LDFLAGS)

dot:	dot.o canvas.o
	$(CC) $^ -o $@ $(LDFLAGS)

# Object files

canvas.o: src/canvas_$(BACKEND).c src/canvas.h
	$(CC) $< -c -o $@ $(CFLAGS)

cursor.o: src/cursor.c src/cursor.h
	$(CC) $< -c -o $@ $(CFLAGS)

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

# Cleanup

.PHONY: clean
clean:
	rm -f $(ALL) *.o
