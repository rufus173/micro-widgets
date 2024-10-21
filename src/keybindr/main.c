#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include "../../lib/input.h"
#include "../../lib/tray.h"

#define MAX_KEY_CODE 200
#define MAX_KEYS_IN_BIND 4

struct key_tracking {
	enum key_state state;
};
struct keybinds {
	int key_count;
	int keycodes[MAX_KEYS_IN_BIND];
	char *command;
};

void run_command(char *);
int un_privilege();
int load_keybinds(struct keybinds **);
void free_keybinds(struct keybinds **, int);
void print_keybinds(struct keybinds *keybinds, int keybind_count);
int main(int argc, char **argv){
	// ------------ setup -------------
	char *keyboard_device_location = get_keyboard_device_location();
	if (*keyboard_device_location == 0){
		fprintf(stderr,"could not find keyboard device file.\n");
		return 1;
	}
	printf("using keyboard %s\n",keyboard_device_location);

	int input_fd = connect_input_fd(keyboard_device_location);
	if (input_fd < 0){
		fprintf(stderr,"could create an input file descriptor. Are you running as root?\n");
		return 1;
	}
	
	//unprivelage one's self to prevent disasters
	if (un_privilege() != 0){
		fprintf(stderr,"could not reduce privelage level. try setting the USER variable to an unprivelaged user\n");
		return 1;
	}

	// --------- setup keybinds -----------
	struct keybinds *keybinds;
	int keybind_count = load_keybinds(&keybinds);
	if (keybind_count < 0){	
		fprintf(stderr,"could not load keybinds. Continuing\n");
	}
	print_keybinds(keybinds,keybind_count);

	// --------- setup key_storage ---------
	struct key_tracking key_storage[MAX_KEY_CODE]; //holds the status of all keys on the board
	for (int i = 0; i < MAX_KEY_CODE; i++){ //setup key_tracking struct list
		key_storage[i].state = RELEASED;
	}

	// ------------ mainloop -----------
	for (;;){
		//get keypress
		struct keypress_info keypress;
		int status = get_keypress(input_fd,&keypress);
		if (status != 0){
			fprintf(stderr,"error occured while trying to read input file\n");
			break;
		}
		
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
	free_keybinds(&keybinds,keybind_count);
	return 0;
}
void run_command(char *command){
	//system(command);
}
int un_privilege(){
	//get users uid and gid
	char *username = getenv("USER");
	if (username == NULL){
		fprintf(stderr,"could not get user from env variables\n");
		perror("getenv");
		return -1;
	}
	printf("using username %s\n",username);
	struct passwd *password = getpwnam(username);
	if (password == NULL){
		fprintf(stderr,"could not get uid info\n");
		perror("getpwnam");
		return -1;
	}

	int status = setgid(password->pw_gid);
	if (status < 0){
		fprintf(stderr,"could not lower privelages: aborting.\n");
		perror("setgid");
		return -1;
	}
	/*status = seteuid(password->pw_uid);
	if (status < 0){
		fprintf(stderr,"could not lower privelages: aborting.\n");
		perror("seteuid");
		return -1;
	}*/
	status = setuid(password->pw_uid);
	if (status < 0){
		fprintf(stderr,"could not lower privelages: aborting.\n");
		perror("setuid");
		return -1;
	}
	printf("using uid %d and gid %d\n",password->pw_uid, password->pw_gid);
	if (password->pw_uid == 0 || password->pw_gid == 0){
		fprintf(stderr,"could not find a non root user to run as.\n");
		return -1;
	}
	return 0;
}
int load_keybinds(struct keybinds **keybinds_to_populate){
	//set up to be allocated
	*keybinds_to_populate = NULL;

	//open config file
	char config_filepath[1024];
	char *user = getenv("USER");
	snprintf(config_filepath,1024,"/home/%s/.config/micro-widgets/keybindr.conf",user);
	printf("checking config file %s\n",config_filepath);
	if (access(config_filepath,R_OK) != 0){
		fprintf(stderr,"could not access file path\n");
		perror("access");
		return -1;
	}
	FILE *config_file = fopen(config_filepath,"r");
	if (config_file == NULL){
		fprintf(stderr,"could not open configfile\n");
		perror("fopen");
		return -1;
	}
	
	int keybind_count = 0;
	//read each line and interpret the config file.
	//discard any incomplete lines.
	for (int line = 0;feof(config_file) == 0;line++){
		//-------- read each line -----------
		char *line_buffer = NULL;
		for (int length = 1;;length++){
			line_buffer = realloc(line_buffer,sizeof(char)*(length+1));
			line_buffer[length] = '\0';
			fread(line_buffer+length-1,1,1,config_file);
			if (feof(config_file) || line_buffer[length-1] == '\n'){
				line_buffer[length-1] = '\0';
				break;
			}
		}
		//ignore empty lines or commented lines
		if (line_buffer[0] == '\0' || line_buffer[0] == '#') goto cleanup_after_line_read;

		//----------- process each line ------------
		//printf("breaking down %s...\n",line_buffer);

		//extract the command as the last arg
		char *command = strrchr(line_buffer,',');// find last instance of comma in string and look at the string after it to the end
		if (command == NULL){
			fprintf(stderr,"invalaid line %d in config: %s\n",line,line_buffer);
			goto cleanup_after_line_read;
		}
		command++; //dont include the comma
		
		//allocate another element
		*keybinds_to_populate = realloc(*keybinds_to_populate,sizeof(struct keybinds)*(keybind_count+1));
		printf("command %s\n",command);
		(*keybinds_to_populate)[keybind_count].command = malloc(sizeof(char) * (strlen(command)+1));
		strcpy((*keybinds_to_populate)[keybind_count].command,command);
		//command is a pointer to something in string line_buffer
		*(command-1) = '\0'; //cut of the command from the line buffer

		//find the last MAX_KEYS_IN_BIND args seperated by commas
		char *key = NULL;
		for (int key_count = 0; key != line_buffer; key_count++){ //repeat untill we reach the begining of the string (key == line_buffer)
			key = strrchr(line_buffer,',');
			if (key == NULL){
				key = line_buffer;
			}else{
				*key = '\0'; //prep for next itteration
				key++; //exclude the , if it exists
			}
			printf("key %s\n",key);
			int key_code = key_to_code(key);
			printf("translated %s to code %d\n",key,key_code);
			(*keybinds_to_populate)[keybind_count].keycodes[key_count] = key_code;
			(*keybinds_to_populate)[keybind_count].key_count = key_count+1;
			if (key_count+1 >= MAX_KEYS_IN_BIND){
				printf("line %d, too many keys\n",line+1);
				break;
			}
		}
		keybind_count++;

		cleanup_after_line_read:
		//free line memory
		free(line_buffer);
	}

	return keybind_count;
}
void free_keybinds(struct keybinds **keybinds,int keybind_count){
	for (int i = 0; i < keybind_count; i++){
		free((*keybinds)[i].command);
	}
	free(*keybinds);
}
void print_keybinds(struct keybinds *keybinds, int keybind_count){
	printf("============== keybinds(%d) ===============\n",keybind_count);
	for (int keybind_index = 0; keybind_index < keybind_count; keybind_index++){
		printf("------- bind %d ------\n",keybind_index);
		printf("command: %s\n",keybinds[keybind_index].command);
		printf("keycode sequence:");
		for (int i = 0; i < keybinds[keybind_index].key_count; i++){
			printf("%d ",keybinds[keybind_index].keycodes[i]);
		}
		printf("\n");
	}
	printf("============== end of keybinds ===============\n");
}
