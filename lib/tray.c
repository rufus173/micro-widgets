#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>// un, un un un un, un un un un un un.
#include <wait.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/tray.socket"
#define BUFFER_SIZE 1024
#define TRAY_VERSION 2

//--------------- structs -----------------
enum opcodes {
	NONE,
	KILL,
	NEW_PROCESS,
	GET_PROCESS_COUNT,
	GET_PROCESSES
};
enum return_code {
	OK,
	BAD_VERSION
};
struct processes {
	int count;
	pid_t *pid;
	char **executable_path;
};
struct handshake {
        enum opcodes opcode;
	int version;
};
struct handshake_response {
        enum return_code status;
};
struct running_processes {
        int count;
        char **executable_path;
};
//prototypes
int start_program(const char *executable_path);
int get_running_program_count();
int check_tray_status();
int get_running_processes(struct running_processes *proc);
int free_running_processes_struct(struct running_processes *proc);
static int respond_handshake(int sock,enum return_code status);
static int connect_tray_socket();
static int serve_requests(int socket); //0 on success 1 on stop signal
static int perform_handshake(int sock, enum opcodes opcode);

//globals
static struct processes proc;

//------------------------ public functions ----------------------
int main_tray(){
	printf("tray starting...\n");
	//set up memory
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
	// -------------------- mainloop --------------
	int serving_requests = 1;
	for (;;){//mainloop
		int client;
		if (serving_requests){// 
			//accept connections
			client = accept(socket_fd,NULL,NULL);
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
		}
		// ------------ serve requests ---------
		int stop = serve_requests(client);
		if (stop){ //stop serving requests
			serving_requests = 0;
		}
		
		// ----------- wait for running forks ---------
		waiting:
		//pid waiting
		for (int i = 0; i < proc.count; i++){
			int status = waitpid(proc.pid[i],NULL,WNOHANG);
			if (status > 0){
				printf("cleaned up fork with pid %d\n",proc.pid[i]);
				//remove from list
				proc.pid[i] = proc.pid[proc.count-1];
				proc.executable_path[i] = proc.executable_path[proc.count-1];
				proc.count--;
				proc.pid = realloc(proc.pid,sizeof(pid_t)*proc.count);
				proc.executable_path = realloc(proc.executable_path,sizeof(char*)*proc.count);
			}
		}
		
		//once all programs accounted for, break from mainloop
		if (stop && proc.count < 1){
			break;
		}

		sleep(0.2); //dont spam the system
	}
	
	printf("tray terminated.\n");
	return 0;
}
int start_program(const char *executable_path){
	//handshake
	int tray = connect_tray_socket();
	if (tray < 0){
		fprintf(stderr,"could not connect to tray.\n");
		return -1;
	}
	int result = perform_handshake(tray,NEW_PROCESS);	
	if (tray < 0){
		fprintf(stderr,"could not handshake tray");
		return -1;
	}

	//send size of command string
	size_t executable_path_length = strlen(executable_path)+1;
	result = write(tray,&executable_path_length,sizeof(size_t));
	if (result < 0){
		fprintf(stderr,"could not send executable_path_length.\n");
		perror("write");
	}

	//send command string
	write(tray,executable_path,executable_path_length);
	if (result < 0){
		fprintf(stderr,"could not send executable_path");
		perror("write");
	}
	
	return 0;
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
int get_running_processes(struct running_processes *proc){
	//setup
	int status = 0;
	proc->count = 0;
	proc->executable_path = NULL; //realloc will allocate this

	//connect and stuff
	int tray = connect_tray_socket();
	if (tray < 0){
		fprintf(stderr,"Could not connect to tray socket. Stop.\n");
		status = -1;
		goto end;
	}
	
	//request running processes
	struct tray_command request;
	int running_count;
	request.opcode = QUERY_RUNNING_EXECUTABLES;
	int result = write(tray,&request,sizeof(struct tray_command));
	if (result < 0){
		fprintf(stderr, "Could not write to socket. Stop.\n");
		perror("write");
		status = -1;
		goto end;
	}
	result = read(tray,&running_count,sizeof(int));
	if (result < 0){
		fprintf(stderr, "Could not get count. Stop.\n");
		perror("read");
		status = -1;
		goto end;
	}
	proc->count = running_count;
	proc->executable_path = (char**)malloc(sizeof(char*)*proc->count);
	for (int i = 0; i < running_count; i++){
		char buffer[BUFFER_SIZE];
		result = read(tray,buffer,sizeof(char)*BUFFER_SIZE);
		if (result < 0){
			fprintf(stderr,"could not read program executable string.\n");
			perror("read");
			status = -1;
			goto end;
		}
		proc->executable_path[i] = (char*)malloc(sizeof(char)*strlen(buffer)+1);
		snprintf(proc->executable_path[i],strlen(buffer)+1,"%s",buffer);
		//printf("%s\n", proc->executable_path[i]);
	}

	//i hate cleaning
	end:
	close(tray);
	return status;
}
int free_running_processes_struct(struct running_processes *proc){
	for (int i = 0; i < proc->count; i++){
		free(proc->executable_path[i]);
	}
	free(proc->executable_path);
	return 0;
}
int check_tray_status(){
	int tray = connect_tray_socket();
	if (tray < 0){
		return -1;
	}

	//check everything is ok
	struct tray_command command;
	struct tray_response response;
	command.opcode = QUERY_RUNNING_COUNT;
	int result = write(tray,&command,sizeof(struct tray_command));
	if (result < 0){
		return -1;
		perror("write");
	}
	result = read(tray, &response, sizeof(struct tray_response));
	if (result < 0){
		return -1;
		perror("read");
	}
	
	//clean and return
	close(tray);
	return 0;
}
//---------------------- private functions ------------------
static int connect_tray_socket(){
	//prepare data
	struct sockaddr_un address;
	memset(&address,0,sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path,SOCKET_PATH,sizeof(address.sun_path)-1);
	
	//connect
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
	return tray;
}
static int perform_handshake(int sock, enum opcodes opcode){
	struct handshake handshake;
	struct handshake_response response;
	memset(&handshake, 0, sizeof(struct handshake));
	memset(&response, 0, sizeof(struct handshake_response));
	handshake.opcode = opcode;
	handshake.version = TRAY_VERSION;

	//start handshake
	int result = write(sock,&handshake,sizeof(struct handshake));
	if (result < 0){
		fprintf(stderr, "could not perform handshake.\n");
		perror("write");
	}

	//get response
	result = read(sock, &response, sizeof(struct handshake_response));
	if (result < 0){
		fprintf(stderr, "could not perform handshake.\n");
		perror("read");
	}

	//check status
	switch(response.status){
		case BAD_VERSION:
			fprintf(stderr,"client tray version mismatch.\n");
			return -1;
		case OK:
			return 0;
	}
}
static int respond_handshake(int sock,enum return_code status){
	struct handshake_response response;
	memset(&response,0,sizeof(struct handshake_response));
	response.status = status;
	int result = write(sock,&response,sizeof(struct handshake_response));
	if (result < 0){
		fprintf(stderr,"could not respond to handshake.\n");
		perror("write");
		return -1;
	}
	return 0;
}
static int serve_requests(int client){
	//serve requests
	struct handshake request;
	memset(&request,0,sizeof(struct handshake));
	request.opcode = NONE; //set a default if no opcode set
	
	result = read(client, &request, sizeof(request));
	if (result < 0){
		fprintf(stderr,"tray: could not read from socket.\n");
		perror("read");
		goto client_cleanup;
	}
	//client version compatibility
	if (request.version != TRAY_VERSION){
		respond_handshake()

	struct tray_response response;
	memset(&response,0,sizeof(struct tray_response));
	response.status = 0;
	switch (request.opcode){
		case NONE: //send an ok and do nothing
			respond_handshake(client,BAD_VERSION);
			goto client_cleanup;
		case KILL: //kill the tray
			respond_handshake(client,OK);
			return 1;
		case RUN_EXECUTABLE:
			//get the command to run
			size_t command_size;
			int result = read(client,&command_size,sizeof(size_t));
			if (result < 0){
				fprintf(stderr,"could not read size of command.\n");
				perror("read");
				goto client_cleanup;
			}
			char *command = malloc(command_size);
			result = read(client, &command, command_size);
			if (result < 0){
				fprintf("could not read command.\n");
				perror("read");
			}

			//fork
			pid_t pid = fork();
			if (pid < 0) perror("fork");
			if (pid == 0){
				response.status = system(command);
			}else{
				proc.count++;
				proc.pid = realloc(proc.pid,sizeof(pid_t)*proc.count);
				proc.pid[proc.count-1] = pid;
				proc.executable_path = realloc(proc.executable_path, sizeof(char *)*proc.count);
				proc.executable_path[proc.count-1] = malloc(sizeof(char) * (strlen(request.executable_path)+1));
				snprintf(proc.executable_path[proc.count-1],strlen(request.executable_path)+1,"%s",request.executable_path);
			}
			//clean
			free(command);
			break;
		case QUERY_RUNNING_COUNT:
			response.count = proc.count;
			response.status = 0;
			break;
		case QUERY_RUNNING_EXECUTABLES:
			int count = proc.count;
			result = write(client,&count,sizeof(int));
			for (int i = 0; i < proc.count; i++){ //transmit all the strings
				char buffer[BUFFER_SIZE];
				snprintf(buffer,1024,"%s",proc.executable_path[i]);
				result = write(client,buffer,sizeof(char)*BUFFER_SIZE);
				if (result < 0){
					fprintf(stderr,"could not write string.\n");
					perror("write");
					goto client_cleanup;
				}
			}
			goto client_cleanup; //skip the response
			break; //just in case
	}

	client_cleanup:
	close(client);
}
