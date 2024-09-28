#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>// un, un un un un, un un un un un un.
#include <wait.h>
#include <sys/socket.h>
#include "tray.h"

#define SOCKET_PATH "/tmp/tray.socket"

struct processes {
	int count;
	pid_t *pid;
};
int new_process();

int main_tray(){
	printf("tray starting...\n");
	//set up memory
	struct processes proc;
	proc.pid = malloc(sizeof(pid_t));
	proc.count = 0;

	//set up socket
	struct sockaddr_un name;
	memset(&name,0,sizeof(struct sockaddr_un));//zero struct
	name.sun_family = AF_UNIX;
	//set up the file location
	strncpy(name.sun_path, SOCKET_PATH, sizeof(name.sun_path)-1/*dont forget the null*/);

	int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socket_fd < 0){
		fprintf(stderr, "tray: could not create socket.\n");
		perror("socket");
		return 1;
	}

	if (access(SOCKET_PATH,F_OK) == 0){
		//if the socket already exists
		remove(SOCKET_PATH);
	}

	//actual creation of socket file
	int result = bind(socket_fd, (struct sockaddr*)&name, sizeof(name));
	if (result < 0){
		fprintf(stderr,"tray: could not bind socket.\n");
		perror("bind");
	}
	
	//start listening for connections
	result = listen(socket_fd, 10);

	for (;;){//mainloop
		//accept connections
		int client = accept(socket_fd,NULL,NULL);

		//serve requests
		//cleanup
		close(client);

		//pid waiting
		for (int i = 0; i < proc.count; i++){
			waitpid(proc.pid[i],NULL,WNOHANG);
		}
	}
	

	return 0;
}
int start_program(const char *executable_path){
	return 0;
}
