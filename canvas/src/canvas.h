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
	// Special ASCII characters
	KEY_ESC = '\033',

	// Non-ASCII
	KEY_LCTRL = 1 << 8,
	KEY_LSHIFT,
	KEY_LALT,
	BTN_LMOUSE,
	BTN_RMOUSE,
	BTN_MMOUSE,

	NUM_BUTTONS // Max index for arrays
};

bool video_start(void);
void setpx(int, int, int);
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
