#include "line1.h"

/*
 *			PLEASE NOTE:
 *
 *	This implementation is intentionally overkill.
 *	Do not take this as a demonstration of good practices.
 */

// Bresenham's line algorithm, parameterized
#define LINE_FUNC(sufx, a, b, cmp, dira, dirb, a1, a0, b1, b0) \
static bool line_##sufx(bool (*f)(int x, int y), int x0, int y0, int x1, int y1) \
{ \
	int d##a = a##a1 - a##a0, d##b = b##b1 - b##b0; \
	d##b <<= 1; \
	int df = d##b - d##a; \
	d##a <<= 1; \
\
	int b = b##0; \
	for (int a = a##0; a cmp a##1; a += dira) { \
		if (f(x, y)) \
			return true; \
		if (df > 0) { \
			b += dirb; \
			df -= d##a; \
		} \
		df += d##b; \
	} \
	return false; \
}

LINE_FUNC(rdg, x, y, <=,  1,  1,  1, 0, 1, 0); // right, down, gradual
LINE_FUNC(rds, y, x, <=,  1,  1,  1, 0, 1, 0); // right, down, steep
LINE_FUNC(rug, x, y, <=,  1, -1,  1, 0, 0, 1); // right, up, gradual
LINE_FUNC(rus, y, x, >=, -1,  1,  0, 1, 1, 0); // right, up, steep

LINE_FUNC(ldg, x, y, >=, -1,  1,  0, 1, 1, 0); // left, down, gradual
LINE_FUNC(lds, y, x, <=,  1, -1,  1, 0, 0, 1); // left, down, steep
LINE_FUNC(lug, x, y, >=, -1, -1,  0, 1, 0, 1); // left, up, gradual
LINE_FUNC(lus, y, x, >=, -1, -1,  0, 1, 0, 1); // left, up, steep

// Calls f on every point in a line, in correct order, until f returns true.
// Returns true if it stopped for this reason.
bool line1(bool (*f)(int,int), int x0, int y0, int x1, int y1)
{
	int dx = x1 - x0, dy = y1 - y0;

	bool right, down, steep;
	right = dx > 0;
	if (!right)
		dx = -dx;
	down = dy > 0;
	if (!down)
		dy = -dy;
	steep = dy > dx;

	int idx = (right << 2) | (down << 1) | (steep << 0);
	static bool (*functree[8])(bool (*)(int,int),int,int,int,int) = {
		line_lug, line_lus, line_ldg, line_lds,
		line_rug, line_rus, line_rdg, line_rds,
	};

	return functree[idx](f, x0, y0, x1, y1);
}
