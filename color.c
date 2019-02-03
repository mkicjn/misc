#include <curses.h>
enum color {
	BOLD=1<<3,
	BLACK=COLOR_BLACK,
	DARK_GRAY=BOLD|COLOR_BLACK,
	RED=COLOR_RED,
	LIGHT_RED=BOLD|COLOR_RED,
	GREEN=COLOR_GREEN,
	LIGHT_GREEN=BOLD|COLOR_GREEN,
	BROWN=COLOR_YELLOW,
	YELLOW=BOLD|COLOR_YELLOW,
	BLUE=COLOR_BLUE,
	LIGHT_BLUE=BOLD|COLOR_BLUE,
	PURPLE=COLOR_MAGENTA,
	PINK=BOLD|COLOR_MAGENTA,
	TEAL=COLOR_CYAN,
	CYAN=BOLD|COLOR_CYAN,
	LIGHT_GRAY=COLOR_WHITE,
	WHITE=BOLD|COLOR_WHITE
};
int color(short f,short b)
{
	static char pairs[1<<6];
	register bool bold=f&BOLD;
	f&=BOLD-1;
	b&=BOLD-1;
	int i=(f<<3)|b;
	if (!pairs[i])
		init_pair(i,f,b);
	return (bold?A_BOLD:0)|COLOR_PAIR(i);
}
void init_rogue(void)
{
	initscr();
	start_color();
	keypad(stdscr,true);
	curs_set(0);
	noecho();
	raw();
}
int main(int argc,char **argv)
{
	init_rogue();
	move(10,10);
	addch('A'|color(CYAN,GREEN));
	getch();
	endwin();
	return 0;
}
