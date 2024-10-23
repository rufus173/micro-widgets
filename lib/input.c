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
static char *search_in_dir(const char *search, const char *dir);

// ------ private functions ---
static char *to_lower_case(char *input){ //non destructive
	char *output = malloc(sizeof(char)*(strlen(input)+1));
	for (int i = 0; i < strlen(input); i++){
		output[i] = tolower(input[i]);
	}
	return output;
}
static char *search_in_dir(const char *search_pattern, const char *dir_to_search){ //ls $dir_to_search | grep -i $search_pattern
	char *result = NULL;
	//ls
	DIR *dir;
	dir = opendir(dir_to_search);
	if (dir == NULL){
		fprintf(stderr,"could not open %s for reading\n",dir_to_search);
		perror("opendir");
		return NULL;
	}
	//for each thing in the dir
	for (struct dirent *file_in_dir = readdir(dir) ; file_in_dir != NULL; file_in_dir = readdir(dir)){
		//search for search_pattern in base_location directory
		//grep -i
		char *lower_case_filename = to_lower_case(file_in_dir->d_name);
		if (strstr(lower_case_filename,search_pattern) != NULL){
			int max_result_size = strlen(dir_to_search)+strlen(file_in_dir->d_name)+2; //dont forget about the slash and '\0'
			result = malloc(sizeof(char)*max_result_size);
			snprintf(result,max_result_size,"%s/%s",dir_to_search,file_in_dir->d_name);
			break;
		}
		free(lower_case_filename);
	}
	closedir(dir);
	return result;
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
	const char *base_location = "/dev/input/by-id";
	const char *search_pattern = "keyboard-event-kbd"; //use lower case

	char *result;
	result = search_in_dir(search_pattern,base_location);
	if (result != NULL){
		return result;
	}

	base_location = "/dev/input/by-path";
	search_pattern = "event-kbd";
	result = search_in_dir(search_pattern,base_location);
	if (result != NULL){
		return result;
	}

	return NULL; //could not find keyboard
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
	}else if (strcmp(key,"alt") == 0){
		return 56;
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
