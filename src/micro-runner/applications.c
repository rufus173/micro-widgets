#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
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
struct applications *get_all_applications(){
	const char *desktop_entry_paths[] = {
		"/usr/share/applications",
		"/usr/local/share/applications",
		"$HOME/.local/share/applications"
	};
	const int desktop_entry_path_count = sizeof(desktop_entry_paths)/sizeof(char *);

	printf("loading applications from %d directories\n",desktop_entry_path_count);
	
	//====== search directories ======
	for (int i = 0; i < desktop_entry_path_count; i++){
		char *directory_location = _env_substitute(desktop_entry_paths[i]);
		printf("loading applications from %s\n",directory_location);

		//cleanup
		free(directory_location);
	}
	//CONFIG_FILE *desktop_file = cfl_load_config_file();
	//cfl_free_config_file(desktop_file);
	return NULL;
}
