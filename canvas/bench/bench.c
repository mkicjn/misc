#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../src/canvas.h"
#include "src/line1.h"
#include "src/line2.h"
#include "src/line3.h"

int color = 0;
bool setpx(int x, int y)
{
	PX(x, y) = color;
	return false;
}

int main()
{
	srand(time(NULL));
	video_start();
	video_update(); 

	int n;
	double t;

	// Benchmark old line function
	n = 0;
	t = 0;
	while (!button_down(KEY_RETURN)) {
		int x0 = rand() % CANVAS_WIDTH;
		int y0 = rand() % CANVAS_HEIGHT;
		int x1 = rand() % CANVAS_WIDTH;
		int y1 = rand() % CANVAS_HEIGHT;
		color = rand() & 0x00ffffff;

		tick();
		line1(setpx, x0, y0, x1, y1);
		t += tock();
		video_update();
		n++;
	}
	printf("%d iterations\n", n);
	printf("Average time (line1): %fms\n", 1000.0 * t / n);
	while (button_down(KEY_RETURN))
		continue;

	// Benchmark new line function
	n = 0;
	t = 0;
	while (!button_down(KEY_RETURN)) {
		int x0 = rand() % CANVAS_WIDTH;
		int y0 = rand() % CANVAS_HEIGHT;
		int x1 = rand() % CANVAS_WIDTH;
		int y1 = rand() % CANVAS_HEIGHT;
		color = rand() & 0x00ffffff;

		tick();
		line2(setpx, x0, y0, x1, y1);
		t += tock();
		video_update();
		n++;
	}
	printf("%d iterations\n", n);
	printf("Average time (line2): %fms\n", 1000.0 * t / n);
	while (button_down(KEY_RETURN))
		continue;

	// Benchmark new LINE macro
	n = 0;
	t = 0;
	while (!button_down(KEY_RETURN)) {
		int x0 = rand() % CANVAS_WIDTH;
		int y0 = rand() % CANVAS_HEIGHT;
		int x1 = rand() % CANVAS_WIDTH;
		int y1 = rand() % CANVAS_HEIGHT;
		int c = rand() & 0x00ffffff;

		tick();
		for LINE(x0, y0, x1, y1)
			PX(x, y) = c;
		t += tock();
		video_update();
		n++;
	}
	printf("%d iterations\n", n);
	printf("Average time (LINE): %fms\n", 1000.0 * t / n);
	while (button_down(KEY_RETURN))
		continue;


	while (!user_quit())
		continue;

	video_stop();

	return 0;
}
