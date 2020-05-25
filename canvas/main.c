#include <stdio.h>
#include "canvas.h"

// Test program

int main()
{
	if (!video_start())
		return 1;

	double const w = 0.57735026919; // = 1.0 / tan(M_PI/3);
	for (int y = 0; y < 480; y++) {
		for (int x = 0; x < 640; x++) {
			double xf = (2.0 *  (double)x/640 - 1.0) * w;
			double yf =  1.0 - ((double)y/480);

			double p_green = yf;
			double p_blue  = 1.0 - (0.5 * (1.0 + yf + xf/w));
			double p_red   = 1.0 - p_green - p_blue;
			if (p_red < -0.0 || p_blue < 0.0) {
				pixel_set(x,y, 0);
				continue;
			}
			unsigned char red   = 255.0 * p_red;
			unsigned char green = 255.0 * p_green;
			unsigned char blue  = 255.0 * p_blue;
			pixel_set(x,y, (red<<16)|(green<<8)|(blue));
		}
	}
	video_update();

	while (!user_quit()) {
		long int balance = 0;
		while (!button_down(KEY_LCTRL) && !button_down(KEY_LALT))
			if (user_quit())
				break;
		while (button_down(KEY_LCTRL) && !button_down(KEY_LALT))
			balance--;
		while (!button_down(KEY_LCTRL) && button_down(KEY_LALT))
			balance++;
		while (button_down(KEY_LCTRL) && button_down(KEY_LALT))
			;
		while (button_down(KEY_LCTRL) && !button_down(KEY_LALT))
			balance--;
		while (!button_down(KEY_LCTRL) && button_down(KEY_LALT))
			balance++;
		printf("Balance: %ld\n", balance);
	}

	video_end();
	return 0;
}
