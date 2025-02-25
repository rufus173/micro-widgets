#include <stdio.h>
#include <stdlib.h>
#include "../config_file_lib.h"
int main(int argc, char **argv){
	if (argc < 2){
		printf("provide the path to the config file\n");
		return EXIT_FAILURE;
	}
	CONFIG_FILE *config_file = cfl_load_config_file(argv[1]);
	if (config_file == NULL){
		perror("cfl_load_config_file");
		return EXIT_FAILURE;
	}
	cfl_free_config_file(config_file);
}
