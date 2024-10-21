#ifndef _INPUT_H
#define _INPUT_H
#include <stdint.h>
enum key_state {
	RELEASED,
	PRESSED,
	HELD,
};
enum special_keys {
	KEY_SUPER = 125,
	KEY_CTRL = 29,
	KEY_SHIFT = 42,
};
struct keypress_info {
	uint16_t keycode;
	enum key_state state;
};
int connect_input_fd(char *location);
char *get_keyboard_device_location();
int get_keypress(int input_fd, struct keypress_info *result);
int key_to_code(char *key);
#endif
