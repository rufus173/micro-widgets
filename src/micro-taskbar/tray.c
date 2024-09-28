#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>// un, un un un un, un un un un un un.
#include <wait.h>
#include <sys/socket.h>
#include "tray.h"
#include <fcntl.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/tray.socket"
#define RUN_EXECUTABLE 1
#define KILL 0

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
		return -1;
	}

	//non blocking accept()
	int result = fcntl(socket_fd, F_SETFL, SOCK_NONBLOCK);
	if (result < 0){
		fprintf(stderr,"tray: could not set fd to be non blocking.\n");
		perror("fcntl");
		return -1;
	}

	if (access(SOCKET_PATH,F_OK) == 0){
		//if the socket already exists
		remove(SOCKET_PATH);
	}

	//actual creation of socket file
	result = bind(socket_fd, (struct sockaddr*)&name, sizeof(name));
	if (result < 0){
		fprintf(stderr,"tray: could not bind socket.\n");
		perror("bind");
		return -1;
	}
	
	//start listening for connections
	result = listen(socket_fd, 10);

	for (;;){//mainloop
		//accept connections
		int client = accept(socket_fd,NULL,NULL);
		if (client < 0){
			if (errno == EAGAIN || errno == EWOULDBLOCK){
				//no connection to accept
				goto waiting;
			}else{
				fprintf(stderr,"tray: could not accept connections.\n");
				perror("accept");
				return -1;
			}
		}

		//serve requests
		struct tray_command request;
		memset(&request,0,sizeof(struct tray_command));
		request.opcode = -1; //set a default
		result = read(client, &request, sizeof(request));
		if (result < 0){
			fprintf(stderr,"tray: could not read from socket.\n");
			perror("read");
			goto client_cleanup;
		}
		struct tray_response response;
		memset(&response,0,sizeof(struct tray_response));
		response.status = 0;
		switch (request.opcode){
			case -1: //something has gone wrong (panic!!!!)
				goto client_cleanup;
			case KILL: //kill the tray
				return 0;
			case RUN_EXECUTABLE: //run executable
				if (access(request.executable_path,X_OK) == 0){
					//go fork yourself
					pid_t pid = fork();
					if (pid < 0) perror("fork");
					if (pid == 0){
						response.status = system(request.executable_path);
					}else{
						proc.count++;
						proc.pid = realloc(proc.pid,sizeof(pid_t)*proc.count);
						proc.pid[proc.count-1] = pid;
					}
				}else{
					response.status = 1;
				}
				break;
		}
		//return a value to them
		result = write(client,&response,sizeof(struct tray_response));
		if (result < 0){
			fprintf(stderr,"could not write to socket.\n");
			perror("write");
			goto client_cleanup;
		}

		client_cleanup:
		close(client);
		
		waiting:
		//pid waiting
		for (int i = 0; i < proc.count; i++){
			int status = waitpid(proc.pid[i],NULL,WNOHANG);
			if (status > 0){
				printf("cleaned up process %d\n",proc.pid[i]);
				//remove from list
				proc.pid[i] = proc.pid[proc.count-1];
				proc.count--;
				proc.pid = realloc(proc.pid,sizeof(pid_t)*proc.count);
			}
		}
	}
	
	printf("tray terminated.\n");
	return 0;
}
int start_program(const char *executable_path){
	struct tray_command command;
	struct tray_response response;
	struct sockaddr_un address;
	memset(&address,0,sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	response.status = 0;

	strncpy(address.sun_path,SOCKET_PATH,sizeof(address.sun_path)-1);

	int tray = socket(AF_UNIX,SOCK_STREAM,0);
	if (tray < 0){
		fprintf(stderr,"could not create socket.\n");
		perror("socket");
		return -1;
	}

	int result = connect(tray,(struct sockaddr *)&address,sizeof(struct sockaddr_un));
	if (result < 0){
		fprintf(stderr,"could not connect to tray.\n");
		perror("connect");
		return -1;
	}

	//prepare data
	command.opcode = RUN_EXECUTABLE;
	snprintf(command.executable_path,1024,"%s",executable_path);
	
	//send data through socket
	result = write(tray,&command,sizeof(struct tray_command));
	if (result < 0){
		fprintf(stderr,"could not write to tray socket.\n");
		perror("write");
		return -1;
	}
	//get response
	result = read(tray,&response,sizeof(struct tray_response));	
	if (result < 0){
		fprintf(stderr,"could not read from tray socket.\n");
		perror("read");
		return -1;
	}
	close(tray);

	return response.status;
}
