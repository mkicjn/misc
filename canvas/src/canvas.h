#ifndef CANVAS_H
#define CANVAS_H

#ifndef SPAN_X
#define SPAN_X 640
#endif
#ifndef SPAN_Y
#define SPAN_Y 480
#endif
#define CANVAS_AREA ((SPAN_X)*(SPAN_Y))

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum button {
	// Special ASCII character keys
	KEY_RETURN = '\r',
	KEY_ESC = '\033',
	KEY_DEL = '\177',

	// Non-ASCII-representable keys
	KEY_LCTRL = 1 << 8,
	KEY_LSHIFT,
	KEY_LALT,
	BTN_LMOUSE,
	BTN_RMOUSE,
	BTN_MMOUSE,

	NUM_BUTTONS // Max index for arrays
};

bool video_start(void);
void video_stop(void);

extern uint32_t *pixels;
#define PX(x, y) pixels[(size_t)(x) + (size_t)(y) * SPAN_X]
void video_update(void);

int mouse_dx(void);
int mouse_dy(void);

bool user_quit(void);
bool button_down(enum button);

void tick(void);
double tock(void);
// ^ Returns time since last tick() in seconds

#endif
