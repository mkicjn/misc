//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <poll.h>
#include <time.h>
#include <limits.h>

/* ******** Time Utilities ******** */

unsigned long msec(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}


/* ******** Screen Utilities ******** */

#define WIDTH 80
#define HEIGHT 23

static inline bool in_bounds(int x, int y)
{
	return (0 <= x && x < WIDTH) && (0 <= y && y < HEIGHT);
}

static inline void cur_pos(int x, int y)
{
	printf("\033[%d;%dH", y+1, x+1);
}

static inline void clear_screen(void)
{
	printf("\033[2J");
}

void screen_init(void)
{
	system("stty raw -echo isig");
	printf("\033[?25l");

	clear_screen();
	cur_pos(0, 0);
}

void screen_deinit(void)
{
	cur_pos(0, HEIGHT);

	printf("\033[?25h");
	system("stty sane");
}


/* ******** Things ******** */

struct thing {
	const char *glyph;
	int x, y;
	void (*think)(struct thing *self);
};

#define MAX_THINGS 20
struct thing things[MAX_THINGS];
int num_things = 0;

struct thing *make_thing(const char *glyph, int x, int y, void (*think)(struct thing *self))
{
	struct thing *t = &things[num_things++];
	t->glyph = glyph;
	t->x = x;
	t->y = y;
	t->think = think;
	return t;
}

struct thing *thing_at(int x, int y)
{
	for (int i = 0; i < num_things; i++)
		if (things[i].x == x && things[i].y == y)
			return &things[i];
	return NULL;
}


/* ******** Drawing ******** */

void draw_glyph(const char *glyph)
{
	printf("%s", glyph);
	fflush(stdout);
}

void draw_thing(struct thing *t)
{
	if (!in_bounds(t->x, t->y))
		return;
	cur_pos(t->x, t->y);
	draw_glyph(t->glyph);
}

const char *ground_glyph(int x, int y)
{
	(void)x;
	(void)y;
	return "\033[0;32m_";
}

void draw_loc(int x, int y)
{
	if (!in_bounds(x, y))
		return;
	cur_pos(x, y);

	struct thing *t = thing_at(x, y);
	if (t)
		draw_glyph(t->glyph);
	else
		draw_glyph(ground_glyph(x, y));
}

void draw_all(void)
{
	cur_pos(0, 0);
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++)
			draw_glyph(ground_glyph(x, y));
		printf("\r\n");
	}

	for (int i = 0; i < num_things; i++)
		draw_thing(&things[i]);
}


/* ******** Signals ******** */

void die(void)
{
	screen_deinit();
	exit(0);
}

volatile bool quit = false;
void signal_handler(int signo)
{
	switch (signo) {
	case SIGINT:
		if (quit)
			die();
		else
			quit = true;
		break;
	case SIGWINCH:
		clear_screen();
		draw_all();
		break;
	default:
		die();
		break;
	}
}

void install_signal_handlers(void)
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGWINCH, signal_handler);
}


/* ******** Main ******** */

unsigned long last_tick = 0;
void tick()
{
	last_tick = msec();

	for (int i = 0; i < num_things; i++)
		things[i].think(&things[i]);
}

bool apply_dir(unsigned char dir, int *xp, int *yp)
{
	static const int dx[UCHAR_MAX] = {
		['1'] = -1, ['y'] = -1,
		['4'] = -1, ['h'] = -1,
		['7'] = -1, ['b'] = -1,
		['3'] =  1, ['u'] =  1,
		['6'] =  1, ['l'] =  1,
		['9'] =  1, ['n'] =  1,
	};
	static const int dy[UCHAR_MAX] = {
		['1'] =  1, ['b'] =  1,
		['2'] =  1, ['j'] =  1,
		['3'] =  1, ['n'] =  1,
		['7'] = -1, ['y'] = -1,
		['8'] = -1, ['k'] = -1,
		['9'] = -1, ['u'] = -1,
	};

	int x = *xp + dx[dir];
	int y = *yp + dy[dir];

	if (in_bounds(x, y)) {
		*xp = x;
		*yp = y;
		return true;
	} else {
		return false;
	}
}

void move_thing(struct thing *t, unsigned char dir)
{
	int x = t->x, y = t->y;
	if (apply_dir(dir, &t->x, &t->y)) {
		draw_loc(x, y);
		draw_thing(t);
	}
}

void inert(struct thing *self)
{
	(void)self;
}

void wander(struct thing *self)
{
	move_thing(self, '1' + rand() % 9);
	// ^ TODO: Remove rand()
}

int main()
{
	install_signal_handlers();
	screen_init();

	struct thing *player = make_thing("\033[0;34m@", WIDTH / 2, HEIGHT / 2, inert);
	make_thing("\033[0;31m@",     WIDTH / 3,     HEIGHT / 3, wander);
	make_thing("\033[0;31m@",     WIDTH / 3, 2 * HEIGHT / 3, wander);
	make_thing("\033[0;31m@", 2 * WIDTH / 3,     HEIGHT / 3, wander);
	make_thing("\033[0;31m@", 2 * WIDTH / 3, 2 * HEIGHT / 3, wander);


	struct pollfd in;
	in.fd = fileno(stdin);
	in.events = POLLIN;

	draw_all();
	unsigned int reflex = 256;
	while (!quit) {
		int input = 0;
		if (poll(&in, 1, reflex) > 0)
			input = getchar();

		switch (input) {
		case 0:
			break;
		case 4:
		case 'q':
			quit = true;
			break;
		default:
			move_thing(player, input);
			break;
		}

		unsigned long now = msec();
		if (now - last_tick > reflex)
			tick();
	}

	die();
	return 0;
}
