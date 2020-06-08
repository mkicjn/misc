#include <stdio.h>
#include "src/canvas.h"

int main()
{
	if (!video_start())
		return 1;

	// Pixel drawing test
	double const w = 0.57735026919; // = 1.0 / tan(M_PI/3);
	for (int y = 0; y < SPAN_Y; y++) {
		for (int x = 0; x < SPAN_X; x++) {
			double xf = (2.0 *  (double)x/SPAN_X - 1.0) * w;
			double yf =  1.0 - ((double)y/SPAN_Y);

			double p_green = yf;
			double p_blue  = 1.0 - (0.5 * (1.0 + yf + xf/w));
			double p_red   = 1.0 - p_green - p_blue;
			if (p_red < 0.0 || p_blue < 0.0) {
				PX(x, y) = 0;
				continue;
			}
			unsigned char red   = 255.0 * p_red;
			unsigned char green = 255.0 * p_green;
			unsigned char blue  = 255.0 * p_blue;
			PX(x, y) = (red<<16)|(green<<8)|(blue);
		}
	}
	video_update();

	while (!user_quit())
		continue;

	video_stop();
	return 0;
}
