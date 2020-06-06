#include "src/canvas.h"
#include "src/line.h"
#include "src/cursor.h"

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

void clear(void)
{
	for (int i = 0; i < CANVAS_WIDTH; i++)
		for (int j = 0; j < CANVAS_HEIGHT; j++)
			PX(i, j) = 0;
}

bool rainbow_px(int x, int y)
{
	static int i = 0;
	PX(x, y) = rainbow(i++);
	return false;
}

int main()
{
	video_start();

	int x = 0, y = 0;
	while (!user_quit()) {
		int oldx = x, oldy = y;
		x = mouse_x();
		y = mouse_y();
		if (button_down(BTN_LMOUSE))
			line(rainbow_px, oldx, oldy, x, y);
		if (button_down(BTN_RMOUSE))
			clear();
		cursor(x, y);
		video_update();
		cursor(x, y);
	}

	video_stop();

	return 0;
}
