#ifndef _CONFIG_FILE_LIB_H
#define _CONFIG_FILE_LIB_H

struct cfl_key_value_pair {
	char *key;
	char *value;
};
struct cfl_config_file_section {
	char *lines;
};
struct cfl_config_file {
	char *location;
	struct cfl_config_file_section *sections_head;
};
typedef struct cfl_config_file CONFIG_FILE;

CONFIG_FILE *cfl_load_config_file(char *location);
int cfl_free_config_file(CONFIG_FILE *config_file);

#endif
