#ifndef _APPLICATIONS_H
#define _APPLICATIONS_H

#include <sys/queue.h>

//====== types and structs ======
struct application {
	char *name;
	char *comment;
	char *exec;
	int terminal;
	LIST_ENTRY(application) next;
};
LIST_HEAD(applications_head, application);

//====== prototypes ======
//returns linked list of broken out .desktop files
struct applications_head *get_all_applications();
void free_applications(struct applications_head *applications_list_head);

#endif
