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

int main()
{
	video_start();

	int newx = 0, newy = 0, i = 0;
	while (!user_quit()) {
		int oldx = newx, oldy = newy;
		newx = mouse_x();
		newy = mouse_y();
		if (button_down(BTN_LMOUSE))
			for LINE(oldx, oldy, newx, newy)
				PX(x, y) = rainbow(i++);
		if (button_down(BTN_RMOUSE))
			clear();
		cursor(newx, newy);
		video_update();
		cursor(newx, newy);
		tick();
		while (tock() < 1.0/144.0)
			continue;
	}

	video_stop();

	return 0;
}
