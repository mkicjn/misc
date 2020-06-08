#include "cursor.h"

#define CLAMP(lo,x,hi) ((x)<(hi)?(lo)>(x)?(lo):(x):(hi))
int mouse_x(void)
{
	static int x = 0;
	x += mouse_dx();
	x = CLAMP(0, x, SPAN_X-1);
	return x;
}
int mouse_y(void)
{
	static int y = 0;
	y += mouse_dy();
	y = CLAMP(0, y, SPAN_Y-1);
	return y;
}

#define INVERT(x) ((x) = ~(x))
void cursor(int x, int y)
{
	static uint32_t xormap[16] = {
		~0,  0,  0,  0,
		 0, ~0, ~0, ~0,
		 0, ~0, ~0,  0,
		 0, ~0,  0, ~0,
	};

	if (x < 0 || y < 0)
		return;

	for (int i = 0; i < 4 && x+i<SPAN_X; i++)
	for (int j = 0; j < 4 && y+j<SPAN_Y; j++) {
		PX(x+i, y+j) ^= xormap[i + j*4];
	}
}
