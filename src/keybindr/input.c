#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <linux/input.h>

#define KEYBOARD_LOCATION_BUFFER_LENGTH 1024

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

// ------ private functions ---

// ------ public functions ----
int connect_input_fd(char *location){
	int input_fd = open(location, O_RDONLY);
	if (input_fd < 0){
		fprintf(stderr,"Could not open input file.\n");
		perror("open");
		return -1;
	}
}
char *get_keyboard_device_location(){
	static char device_location[KEYBOARD_LOCATION_BUFFER_LENGTH] = {0};
	char *base_location = "/dev/input/by-id";
	char *search_pattern = "Keyboard-event-kbd";
	DIR *keyboard_id_dir;

	// implement ls
	keyboard_id_dir = opendir(base_location);
	//for each thing in the dir
	for (struct dirent *file_in_dir = readdir(keyboard_id_dir) ; file_in_dir != NULL; file_in_dir = readdir(keyboard_id_dir)){
		//search for search_pattern in base_location directory
		if (strstr(file_in_dir->d_name,search_pattern) != NULL){
			//printf("match: %s;%s\n",file_in_dir->d_name,search_pattern);
			snprintf(device_location,KEYBOARD_LOCATION_BUFFER_LENGTH,"%s/%s",base_location,file_in_dir->d_name);
		}
	}
	closedir(keyboard_id_dir);

	//return complete location buffer
	return device_location; //device_location[0] will be 0 if no result found
};
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
