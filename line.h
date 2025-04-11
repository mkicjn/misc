#ifndef LINE_H
#define LINE_H

// Bresenham's line algorithm macro
//
// Performs an action on every integer coordinate on a line, in order
// Defines variables x and y in body, allows break and continue

#define LINE(x0, y0, x1, y1) \
/* for */ (int  x = (x0), \
		y = (y0), \
		x1_ = (x1), \
		y1_ = (y1), \
		dx_ = x1_ > x ? x1_ - x : x - x1_, \
		dy_ = y1_ > y ? y1_ - y : y - y1_, \
		sx = x1_ > x ? 1 : -1, \
		sy = y1_ > y ? 1 : -1, \
		err_ = dx_ - dy_ \
		; \
		!(2*err_ >= -dy_ && x ==x1_) && !(2*err_ <= dx_ && y == y1_) \
		; \
		2*err_ >= -dy_ ? err_ -= dy_, x += sx : 0, \
		2*err_ <=  dx_ ? err_ += dx_, y += sy : 0)

#endif
