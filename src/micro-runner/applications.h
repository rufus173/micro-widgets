#ifndef _APPLICATIONS_H
#define _APPLICATIONS_H

//====== types and structs ======
struct application {
	char *name;
	char *comment;
	char *exec;
	int terminal;
	struct applicataion *next;
};
typedef application APP;

//====== prototypes ======
//returns linked list of broken out .desktop files
struct application *get_all_applications();

#endif
