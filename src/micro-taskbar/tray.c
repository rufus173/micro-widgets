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
#define QUERY_RUNNING_COUNT 2
#define QUERY_RUNNING_INDEX 3
#define KILL 0

struct processes {
	int count;
	pid_t *pid;
	char **executable_path;
};
int new_process();
static struct tray_response send_tray_command(struct tray_command command);

int main_tray(){
	printf("tray starting...\n");
	//set up memory
	struct processes proc;
	proc.pid = malloc(sizeof(pid_t));
	proc.count = 0;
	proc.executable_path = malloc(sizeof(char *));

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
						proc.executable_path = realloc(proc.executable_path, sizeof(char *)*proc.count);
						proc.executable_path[proc.count-1] = malloc(sizeof(char) * (strlen(request.executable_path)+1));
						snprintf(proc.executable_path[proc.count-1],strlen(request.executable_path)+1,"%s",request.executable_path);
					}
				}else{
					response.status = 1;
				}
				break;
			case QUERY_RUNNING_COUNT:
				response.count = proc.count;
				response.status = 0;
				break;
			case QUERY_RUNNING_INDEX:
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
				printf("cleaned up fork with pid %d\n",proc.pid[i]);
				//remove from list
				proc.pid[i] = proc.pid[proc.count-1];
				proc.count--;
				proc.pid = realloc(proc.pid,sizeof(pid_t)*proc.count);
			}
		}
		sleep(0.2); //dont spam the system
	}
	
	printf("tray terminated.\n");
	return 0;
}
int start_program(const char *executable_path){
	//set up data structures
	struct tray_command command;

	//prepare data
	command.opcode = RUN_EXECUTABLE;
	snprintf(command.executable_path,1024,"%s",executable_path);
	struct tray_response response = send_tray_command(command);
	return response.status;
}
int get_running_program_count(){
	//set up data structures
	struct tray_command command;

	//prepare data
	command.opcode = QUERY_RUNNING_COUNT;
	struct tray_response response = send_tray_command(command);
	if (response.status < 0){
		return response.status;
	}
	return response.count;
}
static struct tray_response send_tray_command(struct tray_command command){
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
		response.status = -1;
		goto end;
	}

	int result = connect(tray,(struct sockaddr *)&address,sizeof(struct sockaddr_un));
	if (result < 0){
		fprintf(stderr,"could not connect to tray.\n");
		perror("connect");
		response.status = -1;
		goto end;
	}

	//send data through socket
	result = write(tray,&command,sizeof(struct tray_command));
	if (result < 0){
		fprintf(stderr,"could not write to tray socket.\n");
		perror("write");
		response.status = -1;
		goto end;
	}
	//get response
	result = read(tray,&response,sizeof(struct tray_response));	
	if (result < 0){
		fprintf(stderr,"could not read from tray socket.\n");
		perror("read");
		response.status = -1;
		goto end;
	}
	close(tray);

	end:
	return response;
}
