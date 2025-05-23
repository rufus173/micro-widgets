#include "config_file_lib.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
static void _print_config_file(CONFIG_FILE *config_file_data){
	struct cfl_config_file_section *current_section;
	current_section = config_file_data->sections_head;
	for(;current_section != NULL;){
		printf("=== new section [%s] ===\n",current_section->name);
		for (int i = 0; i < current_section->key_value_pair_count; i++){
			printf("[%s] = [%s]\n",
				current_section->key_value_pairs[i].key,
				current_section->key_value_pairs[i].value
			);
		}
		current_section = current_section->next;	
	}
}
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
		
		//====== process line ======
		//new section
		if (formatted_line[0] == '['){
			//find ending tag (same code as before)
			for (int i = 1; i < strlen(formatted_line); i++){
				if (formatted_line[i] == ']'){
					formatted_line[i] = '\0';
					break;
				}
			}
			current_section->next = malloc(sizeof(struct cfl_config_file_section));
			current_section = current_section->next;
			//initialise new section node
			current_section->name = strdup(formatted_line+1);
			current_section->next = NULL;
			current_section->key_value_pairs = NULL;
			current_section->key_value_pair_count = 0;
		//key value pair
		}else{
			//resize array
			current_section->key_value_pair_count++;
			//im abreviating key value pair for my sanity
			int kvp_count = current_section->key_value_pair_count;
			current_section->key_value_pairs = realloc(current_section->key_value_pairs,kvp_count*sizeof(struct cfl_key_value_pair));
			//extract key and value
			char *key = formatted_line;
			char *value = strchr(formatted_line,'=');
			if (value == NULL){ //syntax error
				free(formatted_line);
				fclose(config_file);
				cfl_free_config_file(config_file_data);
				errno = EBADMSG;
				return NULL;
			}
			value[0] = '\0';
			value++;
			current_section->key_value_pairs[kvp_count-1].key = strdup(key);
			current_section->key_value_pairs[kvp_count-1].value = strdup(value);
		}

		cleanup:
		free(formatted_line);
	}

	//====== close the config file ======
	int result = fclose(config_file);
	if (result < 0){
		cfl_free_config_file(config_file_data);
		return NULL;
	}

	//_print_config_file(config_file_data);
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
char *cfl_config_section_get_value(CONFIG_FILE *config_file_data, char *section,char *key){
	//====== search for correct section ======
	for (
	struct cfl_config_file_section *current_section = config_file_data->sections_head;
	current_section != NULL;
	current_section = current_section->next)
	{
		//check if section name matches
		if (strcmp(current_section->name,section) == 0){
			//for each key value pair
			for (int i = 0; i < current_section->key_value_pair_count; i++){
				struct cfl_key_value_pair pair = current_section->key_value_pairs[i];
				//found it
				if (strcmp(pair.key,key) == 0){
					return pair.value;
				}
			}
			//key doesnt exist in said section
			return NULL;
		}else{
			continue;
		}
	}
	//failed to find section
	return NULL;
}
