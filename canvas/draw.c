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

static inline void swap(int *a, int *b)
{
	int c = *a;
	*a = *b;
	*b = c;
}

void clear(void)
{
	for (int i = 0; i < CANVAS_WIDTH; i++)
		for (int j = 0; j < CANVAS_HEIGHT; j++)
			set_pixel(i, j, 0);
}

int rainbow(int i)
{
	int mod = i & 255;
	switch ((i >> 8) % 6) {
	case 0: // Start at red; add green
		return 0x00FF0000 + (mod <<  8);
	case 1: // Start at green/red; subtract red
		return 0x00FFFF00 - (mod << 16);
	case 2: // Start at green; add blue
		return 0x0000FF00 + (mod <<  0);
	case 3: // Start at green/blue; subtract green
		return 0x0000FFFF - (mod <<  8);
	case 4: // Start at blue; add red
		return 0x000000FF + (mod << 16);
	case 5: // Start at blue/red; subtract blue
		return 0x00FF00FF - (mod <<  0);
	}
	return ~0; // unreachable
}

void line(int x0, int y0, int x1, int y1)
{
	static int i = 0;
	if (x1 == x0) {
		if (y1 < y0)
			swap(&y0, &y1);
		for (int y = y0; y <= y1; y++)
			set_pixel(x0, y, rainbow(i++));
		return;
	}
	float m = (float)(y1-y0) / (x1-x0);
	if (m < 1.0 && m > -1.0) { // |slope| < 1
		if (x0 > x1) {
			swap(&x0, &x1);
			swap(&y0, &y1);
		}
		for (int x = x0; x <= x1; x++) {
			int y = y0 + m * (x - x0);
			set_pixel(x, y, rainbow(i++));
		}
	} else { // |slope| > 1
		if (y0 > y1) {
			swap(&x0, &x1);
			swap(&y0, &y1);
		}
		m = 1.0 / m;
		for (int y = y0; y <= y1; y++) {
			int x = x0 + m * (y - y0);
			set_pixel(x, y, rainbow(i++));
		}
	}
}


int main()
{
	video_start();

	int x = 0, y = 0;
	while (!user_quit()) {
		int oldx = x, oldy = y;
		x = clamp(x + mouse_x(), 0, CANVAS_WIDTH);
		y = clamp(y + mouse_y(), 0, CANVAS_HEIGHT);
		if (button_down(BTN_LMOUSE))
			line(oldx, oldy, x, y);
		if (button_down(BTN_RMOUSE))
			clear();
		if (button_down(KEY_ESC))
			break;
		video_update();
	}

	video_stop();

	return 0;
}
