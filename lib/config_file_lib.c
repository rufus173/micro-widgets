#include "config_file_lib.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
static char *read_line(FILE *file){
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

	//====== close the config file ======
	result = fclose(config_file);
	if (result < 0){
		cfl_free_config_file(config_file_data);
		return NULL;
	}


	//====== return to user ======
	return config_file_data;
}
int cfl_free_config_file(CONFIG_FILE *config_file_data){
	//free the struct itself
	free(config_file_data);
}
