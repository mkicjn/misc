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
	KEY_ESC = '\033',
	KEY_W = 'w',
	KEY_A = 'a',
	KEY_S = 's',
	KEY_D = 'd',

	KEY_LCTRL = 1 << 7, // Outside normal ascii
	KEY_LSHIFT,
	KEY_LALT,
	BTN_LMOUSE,
	BTN_RMOUSE,
	BTN_MMOUSE,

	NUM_BUTTONS = 255,
};

bool video_start(void);
void set_pixel(int, int, int);
void video_update(void);
void video_stop(void);

int mouse_x(void);
int mouse_y(void);
int mouse_dx(void);
int mouse_dy(void);
bool button_down(enum button);

bool user_quit(void);

void tick(void);
double tock(void);
// ^ Returns time since last tick() in seconds

#endif
