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
#define SCREEN_HEIGHT 23

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

void screen_init(void)
{
	system("stty raw -echo isig");
	printf("\033[?25l");
	clear_screen();
}

void screen_deinit(void)
{
	cursor_pos(0, SCREEN_HEIGHT);

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

void put_glyph(const char *glyph)
{
	printf("%s", glyph);
	fflush(stdout);
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
	long sector;
} position[MAX_ENTITIES];

static inline long set_position(long e, int x, int y, long sector)
{
	if (e < 0)
		return e;

	position[e].en = true;
	position[e].x = x;
	position[e].y = y;
	position[e].sector = sector;
	return e;
}


/* ******** Sectors ******** */
// TODO: Portals

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
	if (s < 0 || !sector[s].en)
		return false;

	return (0 <= x && x < sector[s].width)
		&& (0 <= y && y < sector[s].height);
}

void draw_sector_background(long s)
{
	if (s < 0 || !sector[s].en || !position[s].en || !appearance[s].en)
		return;

	int x0 = position[s].x, y0 = position[s].y;
	for (int y = y0; y < y0 + sector[s].height; y++) {
		for (int x = x0; x < x0 + sector[s].width; x++) {
			if (cursor_pos(x, y))
				put_glyph(appearance[s].glyph);
		}
	}
}

void draw_sector_entities(long s)
{
	if (s < 0 || !sector[s].en || !position[s].en)
		return;

	for (long e = 0; e < entities; e++) {
		if (!appearance[e].en || !position[e].en || position[e].sector != s)
			continue;
		if (!in_sector(s, position[e].x, position[e].y))
			continue;
		int x = position[s].x + position[e].x;
		int y = position[s].y + position[e].y;
		if (cursor_pos(x, y))
			put_glyph(appearance[e].glyph);
	}
}

void draw_sector(long s)
{
	draw_sector_background(s);
	draw_sector_entities(s);
}

void redraw_all(void)
{
	clear_screen();
	for (int s = 0; s < entities; s++) {
		if (!sector[s].en)
			continue;
		draw_sector(s);
	}
}

void redraw_at(long s, int x, int y)
{
	if (s < 0 || !in_sector(s, x, y))
		return;
	if (!cursor_pos(position[s].x + x, position[s].y + y))
		return;

	long e;
	for (e = 0; e < entities; e++) {
		if (!position[e].en || !appearance[e].en || position[e].sector != s)
			continue;
		if (position[e].sector == s && position[e].x == x && position[e].y == y)
			break;
	}

	if (e < entities)
		put_glyph(appearance[e].glyph);
	else if (appearance[s].en)
		put_glyph(appearance[s].glyph);
	else
		put_glyph("\033[m ");
}

void redraw_entity(long e)
{
	if (e < 0 || !position[e].en || !appearance[e].en)
		return;

	long s = position[e].sector;
	if (s < 0 || !position[s].en)
		return;

	long x = position[s].x + position[e].x;
	long y = position[s].y + position[e].y;
	if (!cursor_pos(x, y))
		return;

	put_glyph(appearance[e].glyph);
}


/* ******** More Components ******** */

// RNG counters
struct {
	//bool en; // (Always enabled)
	uint64_t ctr;
} rng[MAX_ENTITIES];

uint64_t entity_rand(long e)
{
	if (e < 0)
		return 0;

	return cbrng(e, rng[e].ctr++);
}

// Animacy
struct {
	bool en;
	void (*think)(long self);
	int tempo;
	int timer;
} animacy[MAX_ENTITIES];

static inline long set_animacy(long e, void (*think)(long self), int tempo)
{
	if (e < 0)
		return e;

	animacy[e].en = true;
	animacy[e].think = think;
	animacy[e].tempo = tempo;
	animacy[e].timer = tempo - 1;
	return e;
}

void trigger_animacy(void)
{
	for (long e = 0; e < entities; e++) {
		if (!animacy[e].en)
			continue;
		if (animacy[e].timer --> 0)
			continue;
		animacy[e].timer = animacy[e].tempo - 1;
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

	long s = position[e].sector;
	int x = position[e].x, y = position[e].y;
	if (s < 0 || !in_sector(s, x + dx, y + dy))
		return;

	position[e].x += dx;
	position[e].y += dy;

	if (sector[e].en) {
		redraw_all();
	} else {
		redraw_at(s, x, y);
		redraw_entity(e);
	}
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
		redraw_all();
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
	set_position(e, x, y, s);
	return e;
}

long make_wanderer(const char *glyph, int x, int y, long s, int tempo)
{
	long e = make_inert(glyph, x, y, s);
	set_animacy(e, wander, tempo);
	return e;
}

int main()
{
	struct pollfd in;
	in.fd = fileno(stdin);
	in.events = POLLIN;

	long substrate = new_entity();
	set_sector(substrate, 4, 4);
	set_position(substrate, 0, 0, -1);

	long world = new_entity();
	set_sector(world, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 4);
	set_position(world, 2, 2, substrate);
	set_appearance(world, "\033[0;32m\"");
	set_animacy(world, wander, 100);

	make_wanderer("\033[0;31m@",     sector[world].width / 3,     sector[world].height / 3, world, 8);
	make_wanderer("\033[0;31m@", 2 * sector[world].width / 3,     sector[world].height / 3, world, 16);
	make_wanderer("\033[0;31m@",     sector[world].width / 3, 2 * sector[world].height / 3, world, 32);
	make_wanderer("\033[0;31m@", 2 * sector[world].width / 3, 2 * sector[world].height / 3, world, 64);

	long player = make_inert("\033[0;34m@", sector[world].width / 2, sector[world].height / 2, world);

	getrandom(&global_key, sizeof(global_key), 0);
	install_signal_handlers();
	screen_init();
	redraw_all();

	unsigned long reflex = 16;
	unsigned long next_tick = msec() + reflex;
	while (!quit) {
		if (msec() >= next_tick) {
			next_tick += reflex;
			trigger_all();
		}

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
