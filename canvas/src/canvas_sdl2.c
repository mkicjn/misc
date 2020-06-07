#include "canvas.h"
#include <SDL2/SDL.h>

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
			if (ev.key.keysym.sym < 128) { // ASCII keys 
				buttonstate[ev.key.keysym.sym] = down;
				break;
			}
			switch (ev.key.keysym.sym) { // Non-ASCII keys
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
			mousestate.x += ev.motion.xrel;
			mousestate.y += ev.motion.yrel;
			break;
		case SDL_QUIT:
			quitstate = true;
			break;
		}
	}
}

static SDL_Window *window;
static SDL_Surface *surface;
uint32_t *pixels = NULL;
bool video_start(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			CANVAS_WIDTH, CANVAS_HEIGHT, 0);
	surface = SDL_GetWindowSurface(window);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_LockSurface(surface);
	pixels = surface->pixels;
	return window != NULL;
}
void video_update(void)
{
	SDL_UnlockSurface(surface);
	SDL_UpdateWindowSurface(window);
	SDL_LockSurface(surface);
}
void video_stop(void)
{
	SDL_Quit();
}

int mouse_dx(void)
{
	check_events();
	int dx = mousestate.x;
	mousestate.x = 0;
	return dx;
}
int mouse_dy(void)
{
	check_events();
	int dy = mousestate.y;
	mousestate.y = 0;
	return dy;
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
