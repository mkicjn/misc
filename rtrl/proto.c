//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <poll.h>
#include <time.h>
#include <sys/random.h>

/* ******** Pseudo-random Number Generation ******** */

uint64_t splitmix64_ctr(uint64_t key, uint64_t ctr)
{
	uint64_t z = (key + ctr * 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

uint64_t global_key = 0xdeadbeef;
uint64_t cbrng(uint64_t key, uint64_t ctr)
{
	return splitmix64_ctr(global_key + key, ctr);
}

double cbrngf(uint64_t key, uint64_t ctr)
{
	return (cbrng(key, ctr) >> 11) * 0x1.0p-53;
}


/* ******** Time Utilities ******** */

unsigned long msec(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}


/* ******** Screen Utilities ******** */

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 24
#define SCREEN_AREA ((SCREEN_WIDTH) * (SCREEN_HEIGHT))

static inline bool on_screen(int x, int y)
{
	return (0 <= x && x < SCREEN_WIDTH) && (0 <= y && y < SCREEN_HEIGHT);
}

static inline bool cursor_pos(int x, int y)
{
	printf("\033[%d;%dH", y+1, x+1);
	return on_screen(x, y);
}

static inline void clear_screen(void)
{
	printf("\033[2J");
	cursor_pos(0, 0);
}

static char outbuf[SCREEN_AREA * sizeof("\033[38;2;rrr;ggg;bbbm")];
void screen_init(void)
{
	setvbuf(stdout, outbuf, _IOFBF, sizeof(outbuf));
	system("stty raw -echo isig");
	printf("\033[?25l");
	printf("\033[?1049h");
	clear_screen();
}

void screen_deinit(void)
{
	cursor_pos(0, SCREEN_HEIGHT);
	printf("\033[?1049l");
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
	if (entities < MAX_ENTITIES)
		return entities++;
	return -1;
}

// Appearance
struct {
	bool en;
	const char *glyph;
} appearance[MAX_ENTITIES];

static inline long set_appearance(long e, const char *glyph)
{
	if (e < 0)
		return e;

	appearance[e].en = true;
	appearance[e].glyph = glyph;
	return e;
}

// Position
struct {
	bool en;
	int x, y;
} position[MAX_ENTITIES];

static inline long set_position(long e, int x, int y)
{
	if (e < 0)
		return e;

	position[e].en = true;
	position[e].x = x;
	position[e].y = y;
	return e;
}

// Sector
struct {
	bool en;
	int width, height;
} sector[MAX_ENTITIES];

static inline long set_sector(long s, int width, int height)
{
	if (s < 0)
		return s;

	sector[s].en = true;
	sector[s].width = width;
	sector[s].height = height;
	return s;
}

bool in_sector(long s, int x, int y)
{
	if (s < 0 || !sector[s].en || !position[s].en)
		return false;

	int x0 = position[s].x, x1 = x0 + sector[s].width;
	int y0 = position[s].y, y1 = y0 + sector[s].height;
	return ((x0 <= x && x < x1) && (y0 <= y && y < y1));
}

// Bounded
struct {
	bool en;
	long sector;
} bounded[MAX_ENTITIES];

static inline long set_bounded(long e, long s)
{
	if (e < 0 || s < 0 || !sector[s].en)
		return e;

	bounded[e].en = true;
	bounded[e].sector = s;
	return e;
}

// TODO: Portals, to connect sectors


/* ******** Drawing ******** */

const char *drawbuf[SCREEN_WIDTH][SCREEN_HEIGHT];
enum {
	DRAW_EMPTY,
	DRAW_AGAIN,
	DRAW_GLYPH,
} drawstat[SCREEN_WIDTH][SCREEN_HEIGHT];

void flip_pos(int x, int y)
{
	switch (drawstat[x][y]) {
	case DRAW_AGAIN:
		break;
	case DRAW_GLYPH:
		cursor_pos(x, y);
		fputs(drawbuf[x][y], stdout);
		break;
	default:
		cursor_pos(x, y);
		fputs("\033[m ", stdout);
		drawbuf[x][y] = NULL;
		break;
	}
}

void flip(void)
{
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			flip_pos(x, y);
			drawstat[x][y] = DRAW_EMPTY;
		}
	}
	fflush(stdout);
}

void put_glyph(const char *glyph, int x, int y)
{
	if (!on_screen(x, y))
		return;

	if (drawbuf[x][y] == glyph) {
		drawstat[x][y] = DRAW_AGAIN;
	} else {
		drawstat[x][y] = DRAW_GLYPH;
		drawbuf[x][y] = glyph;
	}
}

void draw_entity(long e)
{
	if (e < 0 || !appearance[e].en || !position[e].en)
		return;

	if (sector[e].en) {
		int x = position[e].x, y = position[e].y;
		for (int dy = 0; dy < sector[e].height; dy++)
			for (int dx = 0; dx < sector[e].width; dx++)
				put_glyph(appearance[e].glyph, x + dx, y + dy);
	} else {
		put_glyph(appearance[e].glyph, position[e].x, position[e].y);
	}
}

void draw_all(void)
{
	for (int s = 0; s < entities; s++)
		if (sector[s].en)
			draw_entity(s);

	for (int e = 0; e < entities; e++)
		if (!sector[e].en)
			draw_entity(e);

	flip();
}


/* ******** More Components ******** */

// RNG counters
struct {
	bool en;
	uint64_t ctr;
} rng[MAX_ENTITIES];

static inline long set_rng(long e, uint64_t ctr)
{
	if (e < 0)
		return e;

	rng[e].en = true;
	rng[e].ctr = ctr;
	return e;
}

uint64_t entity_rand(long e)
{
	if (e < 0)
		return 0;

	if (!rng[e].en) // https://xkcd.com/221
		return cbrng(e, rng[e].ctr);

	return cbrng(e, rng[e].ctr++);
}

// Animacy
struct {
	bool en;
	void (*think)(long self);
	int period;
	int timer;
} animacy[MAX_ENTITIES];

static inline long set_animacy(long e, void (*think)(long self), int period)
{
	if (e < 0)
		return e;

	animacy[e].en = true;
	animacy[e].think = think;
	animacy[e].period = period;
	animacy[e].timer = period - 1;
	return e;
}

void trigger_animacy(void)
{
	for (long e = 0; e < entities; e++) {
		if (!animacy[e].en)
			continue;
		if (animacy[e].timer --> 0)
			continue;
		animacy[e].timer = animacy[e].period - 1;
		animacy[e].think(e);
	}
}

void move_entity(long e, unsigned char dir)
{
	if (e < 0 || !position[e].en)
		return;

	int dx = 0, dy = 0;
	apply_dir(dir, &dx, &dy);
	if (dx == 0 && dy == 0)
		return;

	int x = position[e].x + dx;
	int y = position[e].y + dy;
	if (bounded[e].en && !in_sector(bounded[e].sector, x, y))
		return;

	position[e].x = x;
	position[e].y = y;
}

void wander(long self)
{
	move_entity(self, '1' + entity_rand(self) % 9);
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
		//draw_all();
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
	trigger_animacy();
}

long make_inert(const char *glyph, int x, int y, long s)
{
	long e = new_entity();
	set_appearance(e, glyph);
	set_position(e, x, y);
	set_bounded(e, s);
	return e;
}

long make_wanderer(const char *glyph, int x, int y, long s, int period)
{
	long e = make_inert(glyph, x, y, s);
	set_animacy(e, wander, period);
	set_rng(e, 0);
	return e;
}

int main()
{
	struct pollfd in;
	in.fd = fileno(stdin);
	in.events = POLLIN;

	long substrate = new_entity();
	set_position(substrate, 0, 0);
	set_sector(substrate, 4, 4);

	long platform = new_entity();
	set_sector(platform, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 4);
	set_position(platform, 2, 2);
	set_bounded(platform, substrate);
	set_appearance(platform, "\033[0;32m\"");
	set_animacy(platform, wander, 99);
	set_rng(platform, 0);

	make_wanderer("\033[0;31m@",     SCREEN_WIDTH / 3,     SCREEN_HEIGHT / 3, platform, 8);
	make_wanderer("\033[0;33m@", 2 * SCREEN_WIDTH / 3,     SCREEN_HEIGHT / 3, platform, 16);
	make_wanderer("\033[0;35m@",     SCREEN_WIDTH / 3, 2 * SCREEN_HEIGHT / 3, platform, 32);
	make_wanderer("\033[0;36m@", 2 * SCREEN_WIDTH / 3, 2 * SCREEN_HEIGHT / 3, platform, 64);

	long player = make_inert("\033[0;34m@", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, platform);

	getrandom(&global_key, sizeof(global_key), 0);
	install_signal_handlers();
	screen_init();

	unsigned long reflex = 16;
	unsigned long next_tick = msec() + reflex;
	while (!quit) {
		if (msec() >= next_tick) {
			next_tick += reflex;
			trigger_all();
		}

		draw_all();
		int timeout = next_tick - msec();
		if (poll(&in, 1, timeout < 0 ? 0 : timeout) <= 0)
			continue;

		int input = getchar();
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
	}

	die();
	// Unreachable
	return 0;
}
