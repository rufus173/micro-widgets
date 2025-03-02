#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <sys/queue.h>
#include "applications.h"
#include <unistd.h>
#include <sys/stat.h>
#include "config_file_lib.h"

#define PREFERED_TERMINAL "kitty"

//may break in the future but as of now it is NOT IMPLEMENTED FOR SOME REASON
//#define	LIST_PREV(elm, field)		*((elm)->field.le_prev)

static char *_env_substitute(const char *str){
	//====== prepare stuff for later ======
	char *expanded_str = malloc(1);
	expanded_str[0] = '\0';
	int expanded_str_len = 0;


	//====== search for $ character ======
	for (size_t i = 0; i < strlen(str); i++){
		if (str[i] == '$'){ //variable substitution needs to occur
			int variable_start_index = 0;
			int variable_length = 0;
			variable_start_index = i+1;
			//find end of variable
			for (;;){
				i++;
				if ( !(isalpha(str[i]) || (str[i] == '_')) ){//triggers on '\0' at end of string too
					//end of variable
					//slice out the variable name
					char *variable_name = strndup(str+variable_start_index,variable_length);
					//printf("getting [%s]\n",variable_name);
					
					//====== get env variable and append ======
					char *var = getenv(variable_name);
					if (var == NULL){
						//if it isnt set, just sub it for nothing
						break;
					}

					expanded_str_len += strlen(var);
					expanded_str = realloc(expanded_str,expanded_str_len+1);
					strcat(expanded_str,var);

					//variable end has been found
					free(variable_name);
					break;
				}
				variable_length++;
			}
		}
		//append letter to expanded str
		expanded_str[expanded_str_len] = str[i];
		expanded_str_len++;
		expanded_str = realloc(expanded_str,expanded_str_len+1);
		expanded_str[expanded_str_len] = '\0';
	}

	//return (duh)
	return expanded_str;
}
static int _applications_load_from_dir(struct applications_head *applications_list_head,char *directory_location){
	int return_val = 0;
	//====== open the directory ======
	DIR *dir = opendir(directory_location);
	if (dir == NULL){
		//skip if error occurs
		fprintf(stderr,"opendir on [%s]\n",directory_location);
		perror("opendir");
		return -1;
	}

	//====== read all the files ======
	struct dirent *dir_entry = readdir(dir);
	for (;dir_entry != NULL; dir_entry = readdir(dir)){
		//printf("found %s\n",dir_entry->d_name);
		size_t full_path_length = snprintf(NULL,0,"%s/%s",directory_location,dir_entry->d_name) + 1;
		char *full_file_path = malloc(full_path_length);
		snprintf(full_file_path,full_path_length,"%s/%s",directory_location,dir_entry->d_name);
		//====== determine filetype ======
		struct stat path_stats;
		int result = stat(full_file_path,&path_stats);
		if (result < 0){
			perror("stat");
			closedir(dir);
			free(full_file_path);
			return -1;
		}

		char *file_extention = strrchr(dir_entry->d_name,'.');
		if (file_extention == NULL){
			file_extention = "";
		}
		//printf("[%s]",file_extention);

		//is directory
		if (S_ISDIR(path_stats.st_mode) && (strcmp(dir_entry->d_name,".") != 0) && (strcmp(dir_entry->d_name,"..") != 0)){ //check its not '.' or '..'
			
			//hurrah for recursion (add all the files in the new dir)
			printf("searching subdir %s\n",full_file_path);
			int result = _applications_load_from_dir(applications_list_head,full_file_path);
			if (result < 0){
				return_val = -1;
			}
		}
		//is file (and .desktop extention)
		else if(S_ISREG(path_stats.st_mode) && (strcmp(file_extention,".desktop") == 0)){
			//printf("reading config %s\n",full_file_path);
			//====== generate an entry for said file ======
			CONFIG_FILE *config = cfl_load_config_file(full_file_path);
			if (config == NULL){
				printf("config file [%s]",full_file_path);
				perror("cfl_load_config_file");
			}
			char *no_display = cfl_config_section_get_value(config,"Desktop Entry","NoDisplay");

			//check if it should be displayed and ignore if it shouldnt
			if (no_display == NULL || strcmp(no_display,"true") != 0){

				struct application *app = malloc(sizeof(struct application));
				memset(app,0,sizeof(struct application));
				
				//==== fill in values ====
				//name
				char *name = cfl_config_section_get_value(config,"Desktop Entry","Name");
				if (name == NULL){
					app->name = strdup("");
				}else{
					app->name = strdup(name);
				}

				//comment
				char *comment = cfl_config_section_get_value(config,"Desktop Entry","Comment");
				if (comment == NULL){
					app->comment = strdup("");
				}else{
					app->comment = strdup(comment);
				}

				//exec
				char *exec = cfl_config_section_get_value(config,"Desktop Entry","Exec");
				if (exec == NULL){
					app->exec = strdup("");
				}else{
					app->exec = strdup(exec);
				}

				//terminal
				char *terminal = cfl_config_section_get_value(config,"Desktop Entry","Terminal");
				if (terminal == NULL){
					app->terminal = 0;
				}else{
					app->terminal = (strcmp("true",terminal) == 0);
				}


				//insert
				LIST_INSERT_HEAD(applications_list_head,app,next); //O(1) rather then going to the end of the list
			}
			//close the config file
			cfl_free_config_file(config);
		}

		//if its not either do nothing (except free memory)
		free(full_file_path);
	}

	closedir(dir);
	return return_val;
}
static void _print_applications(struct applications_head *applications){
	for (
		struct application *current_application = LIST_FIRST(applications);
		current_application != NULL;
		current_application = LIST_NEXT(current_application,next)
	){
		printf("====== %s ======\n",current_application->name);
		printf("exec = %s\n",current_application->exec);
		printf("terminal = %d\n",current_application->terminal);
	}
}
struct applications_head *get_all_applications(){
	//reverse order priority (priority with duplicate names)
	const char *desktop_entry_paths[] = {
		"/usr/share/applications",
		"/usr/local/share/applications",
		"$HOME/.local/share/applications"
	};
	const int desktop_entry_path_count = sizeof(desktop_entry_paths)/sizeof(char *);

	printf("loading applications from %d directories\n",desktop_entry_path_count);

	//===== initialise linked list to store application data ======
	struct applications_head *applications_list_head = malloc(sizeof(struct applications_head));
	memset(applications_list_head,0,sizeof(struct applications_head));
	LIST_INIT(applications_list_head);
	
	//====== search directories ======
	for (int i = 0; i < desktop_entry_path_count; i++){
		char *directory_location = _env_substitute(desktop_entry_paths[i]);
		printf("loading applications from %s\n",directory_location);
		int result = _applications_load_from_dir(applications_list_head,directory_location);
		if (result < 0){
			fprintf(stderr,"some config files may not have loaded correctly\n");
			perror("_applications_load_from_dir");
		}
		free(directory_location);
	}
	//CONFIG_FILE *desktop_file = cfl_load_config_file();
	//cfl_free_config_file(desktop_file);
	return applications_list_head;
}
void free_applications(struct applications_head *applications_list_head){
	//_print_applications(applications_list_head);
	//do nothing if we are given null
	if (applications_list_head == NULL) return;

	struct application *current_application, *next_application;
	for (current_application = LIST_FIRST(applications_list_head);current_application != NULL;){
		next_application = LIST_NEXT(current_application,next);
		free(current_application->name);
		free(current_application->comment);
		free(current_application->exec);
		free(current_application);
		current_application = next_application;
	}
	free(applications_list_head);
}
size_t get_matching_applications(struct applications_head *app_list_head, struct application *app_buffer,size_t app_buffer_len,char *name ,int flags){
	size_t current_buffer_len = 0;
	//====== linear search ======
	for (
		struct application *current_app = LIST_FIRST(app_list_head);
		current_app != NULL;
		current_app = LIST_NEXT(current_app,next)
	){
		//stop if the buffer is full
		if (current_buffer_len >= app_buffer_len) break;

		if (strncasecmp(current_app->name,name,strlen(name)) == 0){
			//found ; append it to the buffer
			memcpy(app_buffer+current_buffer_len,current_app,sizeof(struct application));
			current_buffer_len++;
		}
	}
	return current_buffer_len;
}
int run_command(struct applications_head *app_list_head,char *command){
	printf("processing command [%s]\n",command);
	//====== extract app details ======
	struct application app_buffer[1];
	size_t app_buffer_len = 1;
	char *extracted_app_name = strdup(command);
	app_buffer_len = get_matching_applications(app_list_head,app_buffer,app_buffer_len,extracted_app_name,0);
	char *command_to_run;
	if (app_buffer_len != 0){
		//====== app name was found ======
		extracted_app_name = strdup(app_buffer[0].exec);
		//filter out percent substitutions e.g. %U %F %d because i dont want to deal with them
		for (size_t i = 0; i < strlen(extracted_app_name); i++){
			if (extracted_app_name[i] == '%'){
				extracted_app_name[i] = ' ';
				if (isalpha(extracted_app_name[i+1])) extracted_app_name[i+1] = ' ';
			}
		}
		//====== launch in a terminal if required ======
		if (app_buffer[0].terminal == 1){
			size_t command_to_run_size = strlen(extracted_app_name)+strlen(PREFERED_TERMINAL)+1+1;
			command_to_run = malloc(command_to_run_size);
			snprintf(command_to_run,command_to_run_size,"%s %s",PREFERED_TERMINAL,extracted_app_name);
		}
	}else{
		//====== app name was not found ======
		command_to_run = strdup(command);
	}
	int result = system(command_to_run);
	free(command_to_run);
	free(extracted_app_name);
	return result;
}
void app_list_insertion_sort(struct applications_head *app_list_head){
	for (
		struct application *current_app = LIST_FIRST(app_list_head);
		current_app != NULL;
	){
		//we need this to move to the next node
		struct application *next_app = LIST_NEXT(current_app,next);

		for (
			struct application *app_to_compare = LIST_FIRST(app_list_head);
			app_to_compare != current_app;
			app_to_compare = LIST_NEXT(app_to_compare,next)
		){
			if ((app_to_compare == NULL) || (strcmp(app_to_compare->name,current_app->name) > 0)){
				//====== delete from old position and re insert ======
				LIST_REMOVE(current_app,next);
				LIST_INSERT_BEFORE(app_to_compare,current_app,next);
				break;
			}
		}
		
		//====== move to the next node ======
		current_app = next_app;
	}
}
