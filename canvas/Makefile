# Parameters

CC = clang
CFLAGS = -O -Wall
LD = ld
LDFLAGS = -lm -lSDL2

BACKEND = sdl2

ALL = grad draw life dot

# Executables

all: $(ALL)

life:	life.o canvas.o
	$(CC) $(LDFLAGS) $^ -o $@

draw:	draw.o canvas.o line.o
	$(CC) $(LDFLAGS) $^ -o $@

grad:	grad.o canvas.o
	$(CC) $(LDFLAGS) $^ -o $@

dot:	dot.o canvas.o
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
