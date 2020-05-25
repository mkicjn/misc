#include "canvas.h"
#include <SDL2/SDL.h>

static bool quitstate = false;
static bool buttonstate[NUM_BUTTONS] = {false};

static void check_events(void)
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		bool down;
		switch (ev.type) {
		case SDL_KEYDOWN: // Fallthrough
		case SDL_KEYUP:
			down = ev.type == SDL_KEYDOWN;
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
		case SDL_QUIT:
			quitstate = true;
			break;
		}
	}
}

bool video_start(void)
{
	SDL_Window *win;
	SDL_Init(SDL_INIT_VIDEO);
	win = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
	return win != NULL;
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
