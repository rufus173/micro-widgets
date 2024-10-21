#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
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
static char *to_lower_case(char *input){ //non destructive
	char *output = malloc(sizeof(char)*(strlen(input)+1));
	for (int i = 0; i < strlen(input); i++){
		output[i] = tolower(input[i]);
	}
	return output;
}

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
	const char *base_location = "/dev/input/by-id";
	const char *search_pattern = "keyboard-event-kbd"; //use lower case
	DIR *keyboard_id_dir;

	// implement ls
	keyboard_id_dir = opendir(base_location);
	//for each thing in the dir
	for (struct dirent *file_in_dir = readdir(keyboard_id_dir) ; file_in_dir != NULL; file_in_dir = readdir(keyboard_id_dir)){
		//search for search_pattern in base_location directory
		char *lower_case_filename = to_lower_case(file_in_dir->d_name);
		if (strstr(lower_case_filename,search_pattern) != NULL){
			//printf("match: %s;%s\n",file_in_dir->d_name,search_pattern);
			snprintf(device_location,KEYBOARD_LOCATION_BUFFER_LENGTH,"%s/%s",base_location,file_in_dir->d_name);
		}
		free(lower_case_filename);
	}
	closedir(keyboard_id_dir);

	//return complete location buffer
	return device_location; //device_location[0] will be 0 if no result found
};
int get_keypress(int input_fd, struct keypress_info *decoded_keypress){
	// ----- get the input -----
	struct input_event input;
	for (;;){//listen untill we get an ACUTAL keyboard event
		int status = read(input_fd,&input,sizeof(struct input_event));
		if (status < 0){
			fprintf(stderr, "could not read from the input file descriptor\n");
			perror("read");
			return -1;
		}
		//actual keyboard event
		if (input.type == EV_KEY){
			break;
		}
	}

	// ----- decode the input -----
	decoded_keypress->state = input.value;
	decoded_keypress->keycode = input.code;
	return 0;
}
int key_to_code(char *key){
	int code = -1;
	if (strcmp(key,"shift") == 0){
		return 54;
	}else if(strcmp(key,"ctrl") == 0){
		return 29;
	}else if(strcmp(key,"super") == 0){
		return 125;
	}
	if (key[1] == '\0'){ //only 1 char
		char translation_table[] = "qwertyuiop[]  asdfghjkl;'   #zxcvbnm,./";
		char *pos = strchr(translation_table,*key);
		if (pos != NULL){
			uintptr_t pointer = translation_table-pos;
			return abs(pointer)+16;
		}
	}
	return code;
}
