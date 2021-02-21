#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "/home/mark/Projects/test/aterm.h"

char player[2][4];
bool *horizontal_lines;
bool *vertical_lines;
int *complete_boxes;
int gridsize;
int current_player = 0;
enum axis {
	HORIZONTAL = 'h',
	VERTICAL = 'v'
};


/*
 *  .  .  .  .
 *  |  |
 *  .__.  .  .
 *
 *  .  .  .  .
 *
 *  .  .  .  .
 *
 */

void draw_dots(int gridsize)
{
	printf(CLS);
	for (int i = 0, x = 0, y = 0; y < gridsize; y++) {
		printf(CUP("%d", "%d") "%d", 2*y+2, x+1, i);
		if (y < gridsize-1)
			printf(CUP("%d", "%d") "%d", 2*y+3, 2*x+1, i);
		i = (i + 1) % 10;
	}
	for (int i = 0, x = 0, y = 0; x < gridsize; x++) {
		printf(CUP("%d", "%d") "%d", y+1, 3*x+2, i);
		if (x < gridsize-1)
			printf(CUP("%d", "%d") "%d%d", y+1, 3*x+3, i, i);
		i = (i + 1) % 10;
	}
	for (int x = 0; x < gridsize; x++)
		for (int y = 0; y < gridsize; y++)
			printf(CUP("%d","%d") ".", 2+2*y, 2+3*x);
	printf(CUP("%d","%d") EL("2"), 2*gridsize + 2, 1);
	fflush(stdout);
}

bool is_box_complete(int x, int y)
{
	// e.g. s2,1 => h2,1 h2,2 v2,1 v3,1
	if (!horizontal_lines[x + y * gridsize])
		return false;
	if (!horizontal_lines[x + (y+1) * gridsize])
		return false;
	if (!vertical_lines[x + y * gridsize])
		return false;
	if (!vertical_lines[(x+1) + y * gridsize])
		return false;
	return true;
}

void draw_line(enum axis a, int x, int y)
{
	switch (a) {
	case HORIZONTAL:
		printf(CUP("%d","%d") "__", 2*y+2, 3*x+3);
		break;
	case VERTICAL:
		printf(CUP("%d","%d") "|", 2*y+1+2, 3*x+2);
		break;
	}
	printf(CUP("%d","%d")EL("2"), 2*gridsize + 2, 1);
	fflush(stdout);
}

void show_initial(int x, int y, int p)
{
	complete_boxes[x + y*(gridsize-1)] = p+1;
	printf(SGR(FG_COLR("%s")), p == 0 ? RED : BLUE);
	printf(CUP("%d","%d") "%s", 2*y+3, 3*x+3, player[p]);
	printf(SGR(RESET));
	printf(CUP("%d","%d") EL("2"), 2*gridsize + 2, 1);
	fflush(stdout);
}

bool make_move(enum axis a, int x, int y)
{
	bool free_turn = false;
	int idx = x + y*gridsize;
	switch (a) {
	case HORIZONTAL:
		if (x >= gridsize-1 || y >= gridsize)
			goto oob;
		if (horizontal_lines[idx])
			goto taken;
		horizontal_lines[idx] = true;
		if (is_box_complete(x, y-1)) {
			show_initial(x, y-1, current_player);
			free_turn = true;
		}
		if (is_box_complete(x, y)) {
			show_initial(x, y, current_player);
			free_turn = true;
		}
		break;
	case VERTICAL:
		if (x >= gridsize || y >= gridsize-1)
			goto oob;
		if (vertical_lines[idx])
			goto taken;
		vertical_lines[idx] = true;
		if (is_box_complete(x-1, y)) {
			show_initial(x-1, y, current_player);
			free_turn = true;
		}
		if (is_box_complete(x, y)) {
			show_initial(x, y, current_player);
			free_turn = true;
		}
		break;
	}
	draw_line(a, x, y);
	return free_turn;
taken:
	printf(CUU("1") EL("2") "Already taken. ");
	return true;
oob:
	printf(CUU("1") EL("2") "Out of bounds. ");
	return true;
}

char *winner = NULL;
bool is_game_over(void)
{
	static bool gameover = false;
	if (gameover)
		return true;
	int p1 = 0, p2 = 0;
	for (int i = 0; i < (gridsize-1)*(gridsize-1); i++) {
		switch (complete_boxes[i]) {
		case 1:
			p1++;
			break;
		case 2:
			p2++;
			break;
		default:
			return false;
		}
	}
	if (p1 > p2)
		winner = player[0];
	else if (p2 > p1)
		winner = player[1];
	gameover = true;
	printf("%s got %d boxes\n", player[0], p1);
	printf("%s got %d boxes\n\n", player[1], p2);
	return true;
}

void take_turn(void)
{
	char a;
	int x, y;
	char buf[80];
	for (;;) {
		printf("%s's turn: ", player[current_player]);
		fflush(stdout);
		fgets(buf, sizeof(buf), stdin);
		if (sscanf(buf, "%c%d,%d", &a, &x, &y) >= 3 && (a == HORIZONTAL || a == VERTICAL))
			break;
		printf("Move format: [h/v]#,#");
		printf(CPL("1") EL("2"));
		printf("Invalid move. ");
		continue;
	}
	if (make_move(a, x, y) && !is_game_over())
		take_turn();
}

int main()
{
	printf("Player 1 initials: ");
	fflush(stdout);
	fgets(player[0], 4, stdin);
	player[0][2] = '\0';
	printf("Player 2 initials: ");
	fflush(stdout);
	fgets(player[1], 4, stdin);
	player[1][2] = '\0';

	for (;;) {
		char buf[80];
		printf("How big of a grid? ");
		fflush(stdout);
		fgets(buf, sizeof(buf), stdin);
		if (sscanf(buf, "%d", &gridsize) > 0)
			break;
	}

	horizontal_lines = calloc(gridsize*gridsize, sizeof(bool));
	vertical_lines = calloc(gridsize*gridsize, sizeof(bool));
	complete_boxes = calloc((gridsize-1)*(gridsize-1), sizeof(int));

	draw_dots(gridsize);

	while (!is_game_over()) {
		take_turn();
		current_player = !current_player;
	}
	if (winner)
		printf("%s wins!\n", winner);
	else
		printf("It was a tie...\n");

	free(horizontal_lines);
	free(vertical_lines);
	free(complete_boxes);

	return 0;
}
