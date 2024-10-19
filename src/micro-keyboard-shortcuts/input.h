#ifndef _INPUT_H
#define _INPUT_H
#include <stdint.h>
enum key_state {
	RELEASED,
	PRESSED,
	HELD,
};
struct keypress_info {
	uint16_t keycode;
	enum key_state state;
};
int connect_input_fd(char *location);
struct keypress_info get_keypress(int input_fd);
#endif
