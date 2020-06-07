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
	static uint32_t buf[CANVAS_AREA] = {0};

	for (int y = 0, i = 0; y < CANVAS_HEIGHT; y++)
	for (int x = 0; x < CANVAS_WIDTH; x++, i++) {
		if (pixels[i] == buf[i])
			continue;
		buf[i] = pixels[i];
		int c = buf[i];
		int r = (c >> 16) & 0xFF;
		int g = (c >>  8) & 0xFF;
		int b = (c >>  0) & 0xFF;
		printf(CUP("%d","%d"), y+1, (x+1)<<1);
		printf(SGR(BG_COLR(CUSTOM RGB("%d","%d","%d"))) "  ",
				r, g, b);
	}
	printf(CUP("%d","1") SGR(RESET), CANVAS_HEIGHT+1);
}

uint32_t *pixels = (uint32_t [CANVAS_AREA]){0};

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
	buttonstate['\r'] = buttonstate['\n'];
}

bool user_quit(void)
{
	update_keys();
	return buttonstate[4]; // ^D
}

bool button_down(enum button b)
{
	update_keys();
	bool ret = buttonstate[b];
	buttonstate[b] = false;
	buttonstate['\n'] = buttonstate['\r'];
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
