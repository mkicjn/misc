#include "src/canvas.h"

static inline int max(int a, int b)
{
	return a > b ? a : b;
}
static inline int min(int a, int b)
{
	return a < b ? a : b;
}
static inline int clamp(int x, int lo, int hi)
{
	return max(lo, min(x, hi-1));
}

void clear(void)
{
	for (int i = 0; i < CANVAS_WIDTH; i++)
		for (int j = 0; j < CANVAS_HEIGHT; j++)
			pixel_set(i,j, 0);
}

int main()
{
	video_start();

	int x = 0, y = 0;
	while (!user_quit()) {
		x = clamp(x + mouse_x(), 0, CANVAS_WIDTH);
		y = clamp(y + mouse_y(), 0, CANVAS_HEIGHT);
		if (button_down(BTN_LMOUSE))
			pixel_set(x,y, ~0);
		if (button_down(BTN_RMOUSE))
			clear();
		video_update();
	}

	video_end();

	return 0;
}
