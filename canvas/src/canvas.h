#ifndef CANVAS_H
#define CANVAS_H

#ifndef CANVAS_WIDTH
#define CANVAS_WIDTH 640
#endif
#ifndef CANVAS_HEIGHT
#define CANVAS_HEIGHT 480
#endif
#define CANVAS_AREA ((CANVAS_WIDTH)*(CANVAS_HEIGHT))

#include <stdbool.h>

enum button {
	KEY_W,
	KEY_A,
	KEY_S,
	KEY_D,
	KEY_LCTRL,
	KEY_LSHIFT,
	KEY_LALT,
	KEY_ESC,

	BTN_LMOUSE,
	BTN_RMOUSE,
	BTN_MMOUSE,

	NUM_BUTTONS
};

bool video_start(void);
void set_pixel(int, int, int);
void video_update(void);
void video_stop(void);

int mouse_x(void);
int mouse_y(void);
bool button_down(enum button);

bool user_quit(void);

void tick(void);
double tock(void);
// ^ Returns time since last tick() in seconds

#endif
