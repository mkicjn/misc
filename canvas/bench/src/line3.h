#ifndef LINE_H
#define LINE_H

// Performs an action on every integer coordinate on a line, in order
// Defines x and y in body, allows break and continue
#define LINE(x0, y0, x1, y1) \
/* for */ (int x = (x0), y = (y0), _x1 = (x1), _y1 = (y1), \
		 _dx = _x1>x ? _x1-x : x-_x1, sx = _x1>x ? 1 : -1, \
	         _dy = _y1>y ? _y1-y : y-_y1, sy = _y1>y ? 1 : -1, \
	         _err = _dx - _dy; \
		 !(2*_err >= -_dy && x ==_x1) && !(2*_err <= _dx && y == _y1); \
		 2*_err>=-_dy?_err-=_dy,x+=sx:0,2*_err<=_dx?_err+=_dx,y+=sy:0)

#endif
