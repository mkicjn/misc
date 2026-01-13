//usr/bin/env tcc $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include "aterm.h"

int main(int argc, char **argv)
{
	int size = 50;
	if (argc > 1)
		sscanf(argv[1], "%d", &size);

	printf(CLS CUP("1","1"));

	double const w = 0.57735026919; // = 1.0 / tan(M_PI/3);
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			double xf = (2.0 *  (double)x/size - 1.0) * w;
			double yf =  1.0 - ((double)y/size);

			double p_green = yf;
			double p_blue  = 1.0 - (0.5 * (1.0 + yf + xf/w));
			double p_red   = 1.0 - p_green - p_blue;
			if (p_red < -0.001 || p_blue < -0.001) {
				printf(SGR(RESET) "  ");
				continue;
			}
			int red   = 255.0 * p_red;
			int green = 255.0 * p_green;
			int blue  = 255.0 * p_blue;
			printf(SGR(BG_COLR(CUSTOM COLR_RGB("%d","%d","%d"))) "  ",
					red, green, blue);
		}
		printf(SGR(RESET) "\n");
	}

	return 0;
}
