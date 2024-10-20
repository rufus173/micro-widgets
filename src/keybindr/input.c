#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

// ------- definitions -------
enum key_state {
	RELEASED,
	PRESSED,
	HELD,
};
struct keypress_info {
	uint16_t keycode;
	enum key_state state;
};

// ------ public functions ----
int connect_input_fd(char *location){
	int input_fd = open(location, O_RDONLY);
	if (input_fd < 0){
		fprintf(stderr,"Could not open input file.\n");
		perror("open");
		return -1;
	}
}
struct keypress_info get_keypress(int input_fd){
	// ----- get the input -----
	struct input_event input;
	for (;;){//listen untill we get an ACUTAL keyboard event
		int status = read(input_fd,&input,sizeof(struct input_event));
		if (status < 0){
			fprintf(stderr, "could not read from the input file descriptor\n");
			perror("read");
		}
		//actual keyboard event
		if (input.type == EV_KEY){
			break;
		}
	}

	// ----- decode the input -----
	struct keypress_info decoded_keypress;
	decoded_keypress.state = input.value;
	decoded_keypress.keycode = input.code;
	return decoded_keypress;
}
