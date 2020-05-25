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

	NUM_BUTTONS
};

bool video_start(void);
void video_end(void);
bool user_quit(void);
bool button_down(enum button);

#endif
