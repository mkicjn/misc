#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "/home/mark/test/aterm.h"

#ifndef WIDTH
#define WIDTH 8
#endif
#ifndef HEIGHT
#define HEIGHT 8
#endif

#define AREA ((WIDTH)*(HEIGHT))
#define XY(x,y) ((x)+(y)*(WIDTH))
#define COUNT(x) (sizeof(x)/sizeof(x[0]))

enum piece {
	PIECE_NONE,
	PIECE_RED,
	PIECE_BLUE
};

enum piece board[AREA];

enum piece current_player = PIECE_NONE;
const char *current_color = NULL;

#define LOG(x) \
	CUP("%d", "0") \
	SGR(RESET AND FG_COLR("%s")) \
	EL("2") \
	x, HEIGHT+2, current_color

void setup_board(void)
{
	printf(CLS CUP("0","0"));
	putchar('0');
	for (int i = 1; i < WIDTH; i++)
		printf("%2d", i);
	putchar('\n');
	printf(SGR(FG_COLR(YELLOW) AND BG_BCOLR(YELLOW)));
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			putchar(' ');
			if (x < WIDTH-1)
				putchar('|');
		}
		putchar('\n');
	}
}

int pick_column(void)
{
	char buf[16];
	printf(LOG("Pick a column: "));
	fflush(stdout);
	fgets(buf, sizeof(buf), stdin);
	return atoi(buf);
}

void next_player(void)
{
	switch (current_player) {
	case PIECE_NONE:
	case PIECE_BLUE:
		current_player = PIECE_RED;
		current_color = RED;
		break;
	case PIECE_RED:
		current_player = PIECE_BLUE;
		current_color = BLUE;
		break;
	}
}

bool drop_piece(int col)
{
	int i;
	if (col < 0 || col >= WIDTH)
		return false;
	for (i = 0; i < HEIGHT; i++)
		if (board[XY(col,i)] == PIECE_NONE)
			break;
	if (i < HEIGHT) {
		board[XY(col,i)] = current_player;
		printf(CUP("%d","%d") SGR(FG_COLR("%s") AND BG_BCOLR(YELLOW)) "0",
				HEIGHT-i+1, 2*col+1, current_color);
		return true;
	}
	return false;
}

void take_turn(void)
{
	int col;
	do {
		col = pick_column();
	} while (!drop_piece(col));
}

int win_ways[][4] = {
	{XY(0,0), XY(1,1), XY(2,2), XY(3,3)},
	{XY(3,0), XY(2,1), XY(1,2), XY(0,3)},
	{XY(0,0), XY(1,0), XY(2,0), XY(3,0)},
	{XY(0,0), XY(0,1), XY(0,2), XY(0,3)},
	{XY(0,3), XY(1,3), XY(2,3), XY(3,3)},
	{XY(3,0), XY(3,1), XY(3,2), XY(3,3)}
};

bool four_in_a_row(void)
{
	for (int x0 = 0; x0 <= WIDTH-4; x0++)
	for (int y0 = 0; y0 <= HEIGHT-4; y0++) {
		int p0 = XY(x0, y0);
		for (int i = 0; i < COUNT(win_ways); i++) {
			enum piece p = PIECE_NONE;
			int streak = 0;
			for (int j = 0; j < 4; j++) {
				int p1 = p0 + win_ways[i][j];
				if (board[p1] == PIECE_NONE) {
					break;
				} else if (board[p1] == p) {
					streak++;
				} else {
					p = board[p1];
					streak = 1;
				}
			}
			if (streak == 4)
				return true;
		}
	}
	return false;
}

bool board_full(void)
{
	for (int i = 0; i < AREA; i++)
		if (board[i] == PIECE_NONE)
			return false;
	return true;
}

int main()
{
	setup_board();

	while (!four_in_a_row()) {
		if (board_full()) {
			current_player = PIECE_NONE;
			break;
		}
		next_player();
		take_turn();
	}

	if (current_player != PIECE_NONE)
		printf(LOG("Winner!" SGR(RESET) "\n"));
	else
		printf(LOG(SGR(RESET) "Draw!\n"));

	return 0;
}
