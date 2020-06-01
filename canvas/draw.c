#include "src/canvas.h"

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

#include <math.h>
void line(int x0, int y0, int x1, int y1)
{ // This is a slow way to do this, but I don't care.
	static int i = 0;
	double dx = x1-x0, dy = y1-y0;
	double m = sqrt(dx*dx + dy*dy);
	dx /= m;
	dy /= m;

	for (double x=x0, y=y0; (x0<x1&&x<x1)||(x0>x1&&x>x1)||(y0<y1&&y<y1)||(y0>y1&&y>y1); x+=dx, y+=dy)
		set_pixel(x, y, rainbow(i++));
}

void clear(void)
{
	for (int i = 0; i < CANVAS_WIDTH; i++)
		for (int j = 0; j < CANVAS_HEIGHT; j++)
			set_pixel(i, j, 0);
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
