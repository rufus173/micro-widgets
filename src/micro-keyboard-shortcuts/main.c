#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "input.h"

#define MAX_KEY_CODE 200

struct key_tracking {
	enum key_state state;
};
struct keybinds {
	int key_count;
	int keycodes[4];
	char *command;
};

void run_command(char *);
int main(int argc, char **argv){
	// ------------ setup -------------
	int input_fd = connect_input_fd("/dev/input/by-id/usb-Logitech_LogiG_TKL_MKeyboard-event-kbd");
	if (input_fd < 0){
		fprintf(stderr,"could create an input file descriptor\n");
		return 1;
	}

	// --------- setup keybinds -----------
	int keybind_count = 1;
	struct keybinds *keybinds;
	keybinds = malloc(sizeof(struct keybinds)*keybind_count);
	//some manual binds
	keybinds[0].key_count = 2; // 2 keys in the sequence
	keybinds[0].keycodes[0] = 125; //windows
	keybinds[0].keycodes[1] = 19; // r key
	keybinds[0].command = "konsole";

	// --------- setup key_storage ---------
	struct key_tracking key_storage[MAX_KEY_CODE]; //holds the status of all keys on the board
	for (int i = 0; i < MAX_KEY_CODE; i++){ //setup key_tracking struct list
		key_storage[i].state = RELEASED;
	}

	// ------------ mainloop -----------
	for (;;){
		//get keypress
		struct keypress_info keypress;
		keypress = get_keypress(input_fd);
		
		//store in the relevant index of the struct list
		key_storage[keypress.keycode].state = keypress.state;

		//check if keybinds have been triggered
		for (int i = 0; i < keybind_count; i++){//for each keybind
			int triggered = 1;
			for (int key_in_bind = 0; key_in_bind < keybinds[i].key_count; key_in_bind++){//for each key in the bind

				//evaluate if any of the keys are not held
				if (key_storage[keybinds[i].keycodes[key_in_bind]].state == RELEASED){
					triggered = 0;
					break;
				}
			}
			if (triggered){
				printf("running command %s\n",keybinds[i].command);
				run_command(keybinds[i].command);
			}
		}

		//print info
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
void run_command(char *command){
	system(command);
}
