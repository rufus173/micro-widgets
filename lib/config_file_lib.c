#include "config_file_lib.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
static char *_read_line(FILE *file){
	char *line = malloc(1);
	size_t line_length = 0;
	line[0] = '\0';
	for (;;){
		int result = fread(line+line_length,1,1,file);
		if (result < 1){
			if (feof(file)) return line;
			if (ferror(file)){
				free(line);
				return NULL;
			}
		}
		line_length++;
		line = realloc(line,line_length+1);
		line[line_length] = '\0';
		if (line[line_length-1] == '\n'){
			line[line_length-1] = '\0';
			return line;
		}
	}
}
CONFIG_FILE *cfl_load_config_file(char *location){
	//====== initialise struct ======
	CONFIG_FILE *config_file_data = malloc(sizeof(CONFIG_FILE));
	memset(config_file_data,0,sizeof(CONFIG_FILE));

	//====== open the config file ======
	FILE *config_file = fopen(location,"r");
	if (config_file == NULL){
		cfl_free_config_file(config_file_data);
		return NULL;
	}

	//====== read the config file ======
	
	//==== read initial section ====
	config_file_data->sections_head = malloc(sizeof(struct cfl_config_file_section));
	struct cfl_config_file_section *current_section = config_file_data->sections_head;
	//find non whitespaced line
	char *line;
	for (;;){
		line = _read_line(config_file);
		if (line == NULL){
			fclose(config_file);
			cfl_free_config_file(config_file_data);
			return NULL;
		}
		if (line[0] == '\0') {free(line);continue;};
		if (line[0] == '#') {free(line);continue;};
		if (line[0] == ';') {free(line);continue;};
		break;
	}
	if (line[0] != '['){ //bad syntax
			fclose(config_file);
			cfl_free_config_file(config_file_data);
			errno = EBADMSG;
			return NULL;
	}
	//find ending tag
	for (int i = 1; i < strlen(line); i++){
		if (line[i] == ']'){
			line[i] = '\0';
			break;
		}
	}
	current_section->name = strdup(line+1);
	current_section->next = NULL;
	current_section->key_value_pairs = NULL;
	current_section->key_value_pair_count = 0;
	free(line);

	//==== read rest of document ====
	for (;;){
		if (feof(config_file)) break;
		char *line = _read_line(config_file);

		//check for errors
		if (line == NULL){
			fclose(config_file);
			cfl_free_config_file(config_file_data);
			return NULL;
		}

		//====== format line ======
		char *formatted_line = strdup(line);
		free(line);

		//discard empty lines and comments
		if (formatted_line[0] == '\0') goto cleanup;
		if (formatted_line[0] == '#') goto cleanup;
		if (formatted_line[0] == ';') goto cleanup;
		
		//start a new section if necessary
		if (formatted_line[0] == '['){
			current_section = malloc(sizeof(struct cfl_config_file_section));

		}
		printf("[%s]\n",formatted_line);

		cleanup:
		free(formatted_line);
	}

	//====== close the config file ======
	int result = fclose(config_file);
	if (result < 0){
		cfl_free_config_file(config_file_data);
		return NULL;
	}


	//====== return to user ======
	return config_file_data;
}
int cfl_free_config_file(CONFIG_FILE *config_file_data){
	//free the linked list of sections
	struct cfl_config_file_section *current_section = config_file_data->sections_head;
	for (;current_section != NULL;){
		free(current_section->name);
		//free all the key-value pairs
		for (int i = 0; i < current_section->key_value_pair_count; i++){
			free(current_section->key_value_pairs[i].key);
			free(current_section->key_value_pairs[i].value);
		}
		free(current_section->key_value_pairs);
		struct cfl_config_file_section *next_section = current_section->next;
		free(current_section);
		current_section = next_section;
	}
	//free the struct itself
	free(config_file_data);
}
