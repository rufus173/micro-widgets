#ifndef _APPLICATIONS_H
#define _APPLICATIONS_H

#include <sys/queue.h>
#include <stddef.h>

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
//give it a buffer of len elements, and it will write into it and return how long the new buffer is
size_t get_matching_applications(struct applications_head *app_list_head, struct application *app_buffer,size_t app_buffer_len,char *name ,int flags);
int run_command(struct applications_head *app_list_head,char *command);
void app_list_insertion_sort(struct applications_head *app_list_head);

#endif
