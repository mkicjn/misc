#include "canvas.h"
#include "aterm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>

static void set_polling_stdin(bool b)
{
	static bool init = false;
	static struct termios old, new;
	if (!init) {
		tcgetattr(STDIN_FILENO, &old);
		new = old;
		new.c_lflag &= ~(ICANON|ECHO);
		new.c_cc[VMIN] = 0;
		new.c_cc[VTIME] = 0;
		init = true;
	}
	tcsetattr(STDIN_FILENO, TCSANOW, b ? &new : &old);
}

static void video_interrupt(int sig)
{
	video_stop();
	exit(sig);
}
bool video_start(void)
{
	printf(SGR(RESET) CUH CLS);
	set_polling_stdin(true);
	signal(SIGINT, video_interrupt);
	return true;
}

void video_update(void)
{
	// Do nothing. Updates are printed immediately.
}

void set_pixel(int x, int y, int c)
{
	int r = (c >> 16) & 0xFF;
	int g = (c >>  8) & 0xFF;
	int b = (c >>  0) & 0xFF;
	printf(CUP("%d","%d"), y+1, (x+1)<<1);
	printf(SGR(BG_COLR(CUSTOM RGB("%d","%d","%d"))) "  ",
			r, g, b);
	printf(CUP("%d","1") SGR(RESET), CANVAS_HEIGHT+1);
}

// TODO: Try to read /dev/input/mice?
int mouse_x(void)
{
	return 0;
}
int mouse_y(void)
{
	return 0;
}

void video_stop(void)
{
	printf(SGR(RESET) CLS CUP("1","1") CUS);
	set_polling_stdin(false);
	signal(SIGINT, SIG_DFL);
}

bool buttonstate[256] = {0};
void update_keys(void)
{
	char c;
	while (read(STDIN_FILENO, &c, 1) > 0)
		buttonstate[c] = true;
}

bool user_quit(void)
{
	update_keys();
	return buttonstate[KEY_ESC];
}

bool button_down(enum button b)
{
	update_keys();
	bool ret = buttonstate[b];
	buttonstate[b] = false;
	return ret;
}

static clock_t last_clock = 0;
void tick(void)
{
	last_clock = clock();
}

double tock(void)
{
	double dt = clock() - last_clock;
	return dt / CLOCKS_PER_SEC;
}
