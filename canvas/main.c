#include <stdio.h>
#include "canvas.h"

// Test program

int main()
{
	if (!video_start())
		return 1;

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
