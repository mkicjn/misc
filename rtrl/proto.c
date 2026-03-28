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

static inline void apply_dir(int dir, int *xp, int *yp)
{
	static const int dx[0x100] = {
		['1'] = -1, ['y'] = -1,
		['4'] = -1, ['h'] = -1,
		['7'] = -1, ['b'] = -1,
		['3'] =  1, ['u'] =  1,
		['6'] =  1, ['l'] =  1,
		['9'] =  1, ['n'] =  1,
	};
	static const int dy[0x100] = {
		['1'] =  1, ['b'] =  1,
		['2'] =  1, ['j'] =  1,
		['3'] =  1, ['n'] =  1,
		['7'] = -1, ['y'] = -1,
		['8'] = -1, ['k'] = -1,
		['9'] = -1, ['u'] = -1,
	};

	*xp += dx[dir & 0xff];
	*yp += dy[dir & 0xff];
}


/* ******** Entities and Basic Components ******** */

#define MAX_ENTITIES 1000
long entities = 0;

static inline long new_entity(void)
{
	return entities++; // TODO: OOM handling
}

// Appearance
struct {
	bool en;
	const char *glyph;
} appearance[MAX_ENTITIES];

static inline void set_appearance(long e, const char *glyph)
{
	appearance[e].en = true;
	appearance[e].glyph = glyph;
}

// Position
struct {
	bool en;
	int x, y;
} position[MAX_ENTITIES];

static inline void set_position(long e, int x, int y)
{
	position[e].en = true;
	position[e].x = x;
	position[e].y = y;
}

long entity_at(int x, int y)
{
	// FIXME: Will return entities that are not visible (or any other desired property)
	//        Need something at least marginally more sophisticated.
	for (long e = 0; e < entities; e++)
		if (position[e].en && position[e].x == x && position[e].y == y)
			return e;
	return -1;
}


/* ******** Drawing ******** */

void draw_glyph(const char *glyph)
{
	printf("%s", glyph);
	fflush(stdout);
}

void draw_entity(long e)
{
	if (!appearance[e].en || !position[e].en)
		return;

	int x = position[e].x, y = position[e].y;
	if (!in_bounds(x, y))
		return;

	cur_pos(x, y);
	draw_glyph(appearance[e].glyph);
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

	long e = entity_at(x, y);
	cur_pos(x, y);
	if (e < 0)
		draw_glyph(ground_glyph(x, y));
	else
		draw_glyph(appearance[e].glyph);
}

void draw_all(void)
{
	cur_pos(0, 0);
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++)
			draw_glyph(ground_glyph(x, y));
		printf("\r\n");
	}

	for (long e = 0; e < entities; e++)
		draw_entity(e);
}


/* ******** More Components ******** */

// Motility
struct {
	bool en;
	void (*style)(long self);
	int tempo;
	int timer;
} motility[MAX_ENTITIES];

void set_motility(long e, void (*style)(long self), int tempo)
{
	motility[e].en = true;
	motility[e].style = style;
	motility[e].tempo = tempo;
	motility[e].timer = tempo;
}

void trigger_motility(void)
{
	for (long e = 0; e < entities; e++)
		if (motility[e].en)
			motility[e].style(e);
}

void move_entity(long e, unsigned char dir)
{
	if (!position[e].en)
		return;

	int dx = 0, dy = 0;
	apply_dir(dir, &dx, &dy);

	int x = position[e].x, y = position[e].y;
	if ((dx || dy) && in_bounds(x + dx, y + dy)) {
		position[e].x += dx;
		position[e].y += dy;
		draw_entity(e);
		draw_loc(x, y);
	}
}

void wander(long self)
{
	// FIXME: An entity with tempo k/2 does not always move at the same time as one at tempo k - why?
	if (!motility[self].en)
		return;
	if (motility[self].timer --> 0)
		return;

	move_entity(self, '1' + rand() % 9); // TODO: Better RNG
	motility[self].timer = motility[self].tempo;
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

void trigger_all(void)
{
	// Trigger all components with runtime behavior
	trigger_motility();
}

long make_inert(const char *glyph, int x, int y)
{
	long e = new_entity();
	set_appearance(e, glyph);
	set_position(e, x, y);
	return e;
}

long make_wanderer(const char *glyph, int x, int y, int tempo)
{
	long e = make_inert(glyph, x, y);
	set_motility(e, wander, tempo);
	return e;
}

int main()
{
	struct pollfd in;
	in.fd = fileno(stdin);
	in.events = POLLIN;

	long player = make_inert("\033[0;34m@", WIDTH / 2, HEIGHT / 2);
	make_wanderer("\033[0;31m@",     WIDTH / 3,     HEIGHT / 3, 4);
	make_wanderer("\033[0;31m@",     WIDTH / 3, 2 * HEIGHT / 3, 8);
	make_wanderer("\033[0;31m@", 2 * WIDTH / 3,     HEIGHT / 3, 16);
	make_wanderer("\033[0;31m@", 2 * WIDTH / 3, 2 * HEIGHT / 3, 32);

	install_signal_handlers();
	screen_init();
	draw_all();

	unsigned long reflex = 16;
	unsigned long next_tick = msec() + reflex;
	while (!quit) {
		int input = 0;
		unsigned long timeout = next_tick - msec();
		if (poll(&in, 1, timeout < 1 ? 1 : timeout) > 0)
			input = getchar();

		switch (input) {
		case 0:
			break;
		case 4: // Fallthrough
		case 'q':
			quit = true;
			break;
		default:
			move_entity(player, input);
			break;
		}

		unsigned long now = msec();
		if (now > next_tick) {
			next_tick = now + reflex;
			trigger_all();
		}
	}

	die();
	// Unreachable
	return 0;
}
