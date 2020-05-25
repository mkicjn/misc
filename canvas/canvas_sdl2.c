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
			switch (ev.key.keysym.sym) {
			case SDLK_w:		buttonstate[KEY_W]	= down; break;
			case SDLK_a:		buttonstate[KEY_A]	= down; break;
			case SDLK_s:		buttonstate[KEY_S]	= down; break;
			case SDLK_d:		buttonstate[KEY_D] 	= down; break;
			case SDLK_LCTRL:	buttonstate[KEY_LCTRL]	= down; break;
			case SDLK_LSHIFT:	buttonstate[KEY_LSHIFT]	= down; break;
			case SDLK_LALT:		buttonstate[KEY_LALT]	= down; break;
			}
			break;
		case SDL_MOUSEBUTTONDOWN: // Fallthrough
		case SDL_MOUSEBUTTONUP:
			down = ev.button.state == SDL_PRESSED;
			switch (ev.button.button) {
			case SDL_BUTTON_LEFT:	buttonstate[BTN_LMOUSE] = down; break;
			case SDL_BUTTON_RIGHT:	buttonstate[BTN_RMOUSE] = down; break;
			case SDL_BUTTON_MIDDLE:	buttonstate[BTN_MMOUSE] = down; break;
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
bool video_start(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
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

void pixel_set(int x, int y, int c)
{
	if (x < 0 || x >= 640 || y < 0 || y >= 480)
		return;
	((int *)surface->pixels)[x + y * surface->w] = c;
}

int mouse_x(void)
{
	int x = mousestate.x;
	mousestate.x = 0;
	return x;
}
int mouse_y(void)
{
	int y = mousestate.y;
	mousestate.y = 0;
	return y;
}

void video_end(void)
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
