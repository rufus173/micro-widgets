#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

struct processes {
	int count
	pid_t *pid;
};
int new_process();

int main_tray(){
	//set up memory
	struct processes proc;
	proc.pid = malloc(sizeof(pid_t));
	proc.count = 0;

	//set up socket

	//pid waiting
	for (int i = 0; i < proc.count; i++){
		waitpid(proc.pid[i],NULL,)
	}
	

	return 0;
}
