#include "canvas.h"
#include "aterm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>

static void set_stdin_blocking(bool block)
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
	tcsetattr(STDIN_FILENO, TCSANOW, block ? &old : &new);
}

static void video_interrupt(int sig)
{
	video_stop();
	exit(sig);
}
bool video_start(void)
{
	printf(SGR(RESET) CUH CLS);
	set_stdin_blocking(false);
	signal(SIGINT, video_interrupt);
	return true;
}

void video_update(void)
{
	// Do nothing. Updates are printed immediately.
}

void setpx(int x, int y, int c)
{
	int r = (c >> 16) & 0xFF;
	int g = (c >>  8) & 0xFF;
	int b = (c >>  0) & 0xFF;
	printf(CUP("%d","%d"), y+1, (x+1)<<1);
	printf(SGR(BG_COLR(CUSTOM RGB("%d","%d","%d"))) "  ",
			r, g, b);
	printf(CUP("%d","1") SGR(RESET), CANVAS_HEIGHT+1);
}

int mouse_x(void)
{
	static int x = 0;
	x += mouse_dx();
	return x;
}
int mouse_y(void)
{
	static int y = 0;
	y += mouse_dy();
	return y;
}
// TODO: Try to read /dev/input/mice?
int mouse_dx(void)
{
	return 0;
}
int mouse_dy(void)
{
	return 0;
}

void video_stop(void)
{
	printf(SGR(RESET) CLS CUP("1","1") CUS);
	set_stdin_blocking(true);
	signal(SIGINT, SIG_DFL);
}

bool buttonstate[256] = {0};
void update_keys(void)
{
	static struct pollfd p = {
		.fd = STDIN_FILENO,
		.events = POLLIN,
	};
	while (poll(&p, 1, 0) > 0) {
		if (p.revents & POLLIN)
			buttonstate[getchar()] = true;
		else
			break;
	}
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
