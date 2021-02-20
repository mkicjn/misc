#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct soko {
	int w, h;
	int p_x, p_y;
	char board[];
};

struct soko *read_file(FILE *f)
{
	static char buf[1024];
	int w, h;
	struct soko *s;
	fgets(buf, sizeof(buf), f);
	if (sscanf(buf, "%d, %d\n", &w, &h) < 2)
		return NULL;
	w += 2; // Leave room for "\n\0"
	s = malloc(sizeof(struct soko) + w*h);
	s->w = w;
	s->h = h;
	for (int i = 0; i < h; i++)
		fgets(&s->board[i*w], w, f);
	for (int i = 0; i < w*h; i++) {
		if (s->board[i] == '@') {
			s->p_x = i % w;
			s->p_y = i / w;
			break;
		}
	}
	return s;
}

void print_board(struct soko *s)
{
	int w = s->w, h = s->h;
	printf("\033[1;1H"); // Reset cursor position
	for (int i = 0; i < h; i++)
		printf("%*s\r", (int)w, &s->board[i*w]);
	putchar('\n');
}

enum mov_res {
	MOV_QUIT,
	MOV_PUSH,
	MOV_SUCCESS,
	MOV_FAILURE
};

enum mov_res move(struct soko *s, char key)
{
	int from, to;
	int dx = 0, dy = 0;
	enum mov_res status;
	if (key < 0 || key == 4 || key == 'q') // Exit (q or ^D)
		return MOV_QUIT;
	if ('1' <= key && key <= '9') { // Movement
		int n = key - '0';
		dx =   (n-1) % 3 - 1;
		dy = -((n-1) / 3 - 1);
	}
	if (dx == 0 && dy == 0) // Do nothing
		return MOV_FAILURE;
	from =  s->p_y     * s->w +  s->p_x;
	to   = (s->p_y+dy) * s->w + (s->p_x+dx);
	switch (s->board[to]) {
	case '.': // Empty
		if (s->board[to-dy*s->w] != '.' && s->board[to-dx] != '.')
			return MOV_FAILURE; // No squeezing between things
		s->p_x += dx;
		s->p_y += dy;
do_move:	s->board[to] = s->board[from];
		s->board[from] = '.';
		return MOV_SUCCESS;
	case '0': // Boulder
		if (s->board[from] != '@') // Non-player push
			return MOV_FAILURE;
		if ((key - '0' & 1) == 1) // Diagonal push
			return MOV_FAILURE;
		s->p_x += dx;
		s->p_y += dy;
		status = move(s, key);
		s->p_x -= dx;
		s->p_y -= dy;
		if (status == MOV_SUCCESS)
			goto do_move;
		else
			return MOV_FAILURE;
	case '^': // Goal
		if (s->board[from] == '0') { // Boulder move to goal
			s->p_x += dx;
			s->p_y += dy;
			s->board[from] = '.';
			goto do_move;
		}
		return MOV_FAILURE;
	default: // Wall
		return MOV_FAILURE;
	}
}

int main(int argc, char **argv)
{
	FILE *f;
	struct soko *s;
	if (argc < 2)
		return 1;
	f = fopen(argv[1], "r");
	if (f == NULL)
		return 1;
	s = read_file(f);
	system("stty raw");
	printf("\033[2J"); // Clear screen
	for (;;) {
		enum mov_res status;
		char c;
		print_board(s);
		c = getchar();
		status = move(s, c);
		switch (status) {
		case MOV_SUCCESS:
		case MOV_PUSH:
			fputc(c, stderr);
			break;
		case MOV_FAILURE:
			continue;
		case MOV_QUIT:
			system("stty cooked");
			return 0;
		}
	}
}
