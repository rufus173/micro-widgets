#ifndef TRAY_H
#define TRAY_H
//main_tray() flags
#define TRAY_NO_PERSIST 1
struct running_processes {
	int count;
	char **executable_path;
};
int main_tray(int options);
int start_program(const char *executable_path);
int get_running_program_count();
int get_running_processes(struct running_processes *proc);
int free_running_processes_struct(struct running_processes *proc);
int check_tray_status();//0 for ok -1 for not alive
#endif
