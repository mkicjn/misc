#include "line2.h"

// Calls f on every point in a line, in correct order, until f returns true.
// Returns true if it stopped for this reason.
bool line2(bool (*f)(int x, int y), int x0, int y0, int x1, int y1)
{
	int dx = x1 - x0, sx = 1;
	if (dx < 0) {
		dx = -dx;
		sx = -sx;
	}
	int dy = y1 - y0, sy = 1;
	if (dy < 0) {
		dy = -dy;
		sy = -sy;
	}
	int err = dx - dy;

	for (;;) {
		if (f(x0, y0))
			return true;
		if (err * 2 >= -dy) {
			if (x0 == x1)
				break;
			err -= dy;
			x0 += sx;
		}
		if (err * 2 <= dx) {
			if (y0 == y1)
				break;
			err += dx;
			y0 += sy;
		}
	}
	return false;
}
