#include "canvas.h"
#include <SDL2/SDL.h>
#include <stdint.h>

static bool quitstate = false;
static bool buttonstate[NUM_BUTTONS] = {false};
static struct {int x, y;} mousestate = {.x = 0, .y = 0};

static void check_events(void)
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		bool down;
		switch (ev.type) {
		case SDL_KEYDOWN: // Fallthrough
		case SDL_KEYUP:
			down = ev.key.state == SDL_PRESSED;
			if (ev.key.keysym.sym < 128) {
				buttonstate[ev.key.keysym.sym] = down;
				break;
			}
			switch (ev.key.keysym.sym) {
			case SDLK_LCTRL:	buttonstate[KEY_LCTRL]	= down; break;
			case SDLK_LSHIFT:	buttonstate[KEY_LSHIFT]	= down; break;
			case SDLK_LALT:		buttonstate[KEY_LALT]	= down; break;
			case SDLK_ESCAPE:	buttonstate[KEY_ESC]	= down; break;
			}
			break;
		case SDL_MOUSEBUTTONDOWN: // Fallthrough
		case SDL_MOUSEBUTTONUP:
			down = ev.button.state == SDL_PRESSED;
			switch (ev.button.button) {
			case SDL_BUTTON_LEFT:	buttonstate[BTN_LMOUSE]	= down; break;
			case SDL_BUTTON_RIGHT:	buttonstate[BTN_RMOUSE]	= down; break;
			case SDL_BUTTON_MIDDLE:	buttonstate[BTN_MMOUSE]	= down; break;
			}
			break;
		case SDL_MOUSEMOTION:
			mousestate.x = ev.motion.x;
			mousestate.y = ev.motion.y;
			break;
		case SDL_QUIT:
			quitstate = true;
			break;
		}
	}
}

static SDL_Window *window;
static SDL_Surface *surface;
bool video_start(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			CANVAS_WIDTH, CANVAS_HEIGHT, 0);
	surface = SDL_GetWindowSurface(window);
	SDL_LockSurface(surface);
	return window != NULL;
}

void video_update(void)
{
	SDL_UnlockSurface(surface);
	SDL_UpdateWindowSurface(window);
	SDL_LockSurface(surface);
}

void set_pixel(int x, int y, int c)
{
	if (x < 0 || x >= CANVAS_WIDTH || y < 0 || y >= CANVAS_HEIGHT)
		return;
	((int *)surface->pixels)[x + y * surface->w] = c;
}

int mouse_x(void)
{
	return mousestate.x;
}
int mouse_y(void)
{
	return mousestate.y;
}
int mouse_dx(void)
{
	static int x0 = 0;
	int dx = mouse_x() - x0;
	x0 += dx;
	return dx;
}
int mouse_dy(void)
{
	static int y0 = 0;
	int dy = mouse_y() - y0;
	y0 += dy;
	return dy;
}

void video_stop(void)
{
	SDL_Quit();
}

bool user_quit(void)
{
	check_events();
	return quitstate;
}

bool button_down(enum button b)
{
	check_events();
	return buttonstate[b];
}

static uint64_t last_tick = 0;
void tick(void)
{
	last_tick = SDL_GetPerformanceCounter();
}

double tock(void)
{
	double dt = SDL_GetPerformanceCounter() - last_tick;
	return dt / SDL_GetPerformanceFrequency();
}
