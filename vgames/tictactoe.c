#include <stdio.h>
#include <stdbool.h>

static const int winmat[][3] = {
	{7,8,9}, {4,5,6}, {1,2,3},
	{7,4,1}, {8,5,2}, {9,6,3},
	{1,5,9}, {7,5,3}
};

#define COUNT(X) (sizeof(X)/sizeof(*X))
const int n_win_ways = COUNT(winmat);

typedef char Player;
enum player {
	NONE = ' ',
	X = 'X',
	O = 'O'
};

typedef struct {
	Player pos[10];
} Board;

Player check_win(Board *b)
{
	// For each way of winning
	for (int i = 0; i < n_win_ways; i++) {
		Player space[3];
		// Keep track of moves in each place
		for (int j = 0; j < 3; j++)
			space[j] = b->pos[winmat[i][j]];
		// If it's all the same, that player wins
		if (space[0] == NONE)
			continue;
		else if (space[0] == space[1] && space[1] == space[2])
			return space[0];
	}
	return NONE;
}

void print_board(Board *b)
{
	printf(" %c|%c|%c \n", b->pos[7], b->pos[8], b->pos[9]);
	printf("-------\n");
	printf(" %c|%c|%c \n", b->pos[4], b->pos[5], b->pos[6]);
	printf("-------\n");
	printf(" %c|%c|%c \n", b->pos[1], b->pos[2], b->pos[3]);
}

bool moves_left(Board *b)
{
	for (int i = 1; i < 10; i++)
		if (b->pos[i] == NONE)
			return true;
	return false;
}

#define IN_BUF_SIZE 80

int main()
{
	// Instantiate board
	Board b;
	for (int i = 0; i < 10; i++)
		b.pos[i] = NONE;
	// Input loop
	char input_buf[IN_BUF_SIZE];
	Player cur_move = NONE;
	print_board(&b);
	while (moves_left(&b) && fgets(input_buf, sizeof(input_buf), stdin)) {
		// Scan input
		int p;
		if (sscanf(input_buf, "%d", &p) < 1)
			continue;
		// Bounds check
		if (!(1 <= p && p <= 9))
			continue;
		if (b.pos[p] != NONE)
			continue;
		// Alternate turn
		if (cur_move == X)
			cur_move = O;
		else
			cur_move = X;
		// Make move
		b.pos[p] = cur_move;
		// Print board
		print_board(&b);
		// Check winner
		Player winner = check_win(&b);
		if (winner != NONE) {
			printf("Winner: %c\n", cur_move);
			break;
		}
	}
	return 0;
}
