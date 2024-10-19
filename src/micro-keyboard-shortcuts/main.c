#include <stdio.h>
#include <unistd.h>
#include "input.h"
int main(int argc, char **argv){
	// ------------ setup -------------
	int input_fd = connect_input_fd("/dev/input/by-id/usb-Logitech_LogiG_TKL_MKeyboard-event-kbd");
	if (input_fd < 0){
		fprintf(stderr,"could create an input file descriptor\n");
		return 1;
	}
	// ------------ mainloop -----------
	for (;;){
		struct keypress_info keypress;
		keypress = get_keypress(input_fd);
		printf("key with code %d ",keypress.keycode);
		switch (keypress.state){
		case PRESSED:
			printf("pressed\n");
			break;
		case RELEASED:
			printf("released\n");
			break;
		case HELD:
			printf("held\n");
			break;
		}
	}
	// program ends
	close(input_fd);
	return 0;
}
