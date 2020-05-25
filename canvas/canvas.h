#ifndef CANVAS_H
#define CANVAS_H

#include <stdbool.h>

enum button {
	KEY_W,
	KEY_A,
	KEY_S,
	KEY_D,
	KEY_LCTRL,
	KEY_LSHIFT,
	KEY_LALT,

	BTN_LMOUSE,
	BTN_RMOUSE,
	BTN_MMOUSE,

	NUM_BUTTONS
};

bool video_start(void);
void pixel_set(int, int, int);
void video_update(void);
void video_end(void);

int mouse_x(void);
int mouse_y(void);
bool button_down(enum button);

bool user_quit(void);

#endif
