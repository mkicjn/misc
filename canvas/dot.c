#include <stdio.h>
#include "src/canvas.h"

#define CLAMP(lo,n,hi) ((n)<(hi)?(lo)>(n)?(lo):(n):(hi))

int main()
{
	video_start();

	double px = CANVAS_WIDTH/2, py = CANVAS_HEIGHT/2;

	tick();
	while (!user_quit()) {
		double speed = 1e8/CANVAS_AREA;
		double dt = tock();
		tick();

		PX(px, py) = 0;

		// Collect input
		int dx = 0, dy = 0;
		dy -= button_down('w');
		dx -= button_down('a');
		dy += button_down('s');
		dx += button_down('d');

		// Apply speed and direction
		#define R2O2 0.707107
		if (dx && dy) {
			px += dx * speed * dt * R2O2;
			py += dy * speed * dt * R2O2;
		} else if (dx) {
			px += dx * speed * dt;
		} else if (dy) {
			py += dy * speed * dt;
		}

		// Bounds checking
		px = CLAMP(0, px, CANVAS_WIDTH-1);
		py = CLAMP(0, py, CANVAS_HEIGHT-1);

		PX(px, py) = ~0;

		video_update();
	}

	video_stop();
	return 0;
}
