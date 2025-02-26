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
	//====== open the directory ======
	DIR *dir = opendir(directory_location);
	if (dir == NULL){
		//skip if error occurs
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

		//is directory
		if (S_ISDIR(path_stats.st_mode) && (strcmp(dir_entry->d_name,".") != 0) && (strcmp(dir_entry->d_name,"..") != 0)){ //check its not '.' or '..'
			
			//hurrah for recursion (add all the files in the new dir)
			printf("searching subdir %s\n",full_file_path);
			_applications_load_from_dir(applications_list_head,full_file_path);
		}
		//is file
		else if(S_ISREG(path_stats.st_mode)){
			//====== generate an entry for said file ======
			struct application *app = malloc(sizeof(struct application));
			memset(app,0,sizeof(struct application));
			CONFIG_FILE *config = cfl_load_config_file(full_file_path);
			
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
			char *terminal = cfl_config_section_get_value(config,"Desktop Entry","Name");
			if (name == NULL){
				app->terminal = 0;
			}else{
				app->terminal = (strcmp("true",terminal) == 0);
			}

			cfl_free_config_file(config);

			//insert
			LIST_INSERT_HEAD(applications_list_head,app,next); //O(1) rather then going to the end of the list
		}

		//if its not either do nothing (except free memory)
		free(full_file_path);
	}

	closedir(dir);
	return 0;
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
		_applications_load_from_dir(applications_list_head,directory_location);
		free(directory_location);
	}
	//CONFIG_FILE *desktop_file = cfl_load_config_file();
	//cfl_free_config_file(desktop_file);
	return NULL;
}
void free_applications(struct applications_head *applications_list_head){
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
