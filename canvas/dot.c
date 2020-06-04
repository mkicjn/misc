#include <stdio.h>
#include "src/canvas.h"

int main()
{
	video_start();

	double px = CANVAS_WIDTH/2, py = CANVAS_HEIGHT/2;

	tick();
	while (!user_quit()) {
		double speed = 100.0;
		double dt = tock();
		tick();

		set_pixel(px, py, 0);

		if (button_down(KEY_LSHIFT))
			speed *= 2;
		if (button_down('w'))
			py -= speed * dt;
		if (button_down('a'))
			px -= speed * dt;
		if (button_down('s'))
			py += speed * dt;
		if (button_down('d'))
			px += speed * dt;

		if (button_down(BTN_LMOUSE)) {
			px = mouse_x();
			py = mouse_y();
		}

		if (px < 0)
			px = 0;
		else if (px >= CANVAS_WIDTH)
			px = CANVAS_WIDTH-1;
		if (py < 0)
			py = 0;
		else if (py >= CANVAS_HEIGHT)
			py = CANVAS_HEIGHT-1;

		set_pixel(px, py, ~0);
		video_update();
	}

	video_stop();
	return 0;
}
