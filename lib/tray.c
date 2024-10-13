#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
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
	int stop = 0;
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
		stop = serve_requests(client);
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
	result = write(tray,executable_path,executable_path_length);
	if (result < 0){
		fprintf(stderr,"could not send executable_path");
		perror("write");
	}
	close(tray);
	return 0;
}
int get_running_program_count(){
	//handshake
	int tray = connect_tray_socket();
	if (tray < 0){
		fprintf(stderr,"could not connect to tray.\n");
		return -1;
	}
	int result = perform_handshake(tray,GET_PROCESS_COUNT);
	if (result < 0){
		fprintf(stderr,"could not perform handshake");
		return -1;
	}

	//get count
	int count;
	result = read(tray,&count,sizeof(int));
	if (result < 0){
		fprintf(stderr,"could not read process count.\n");
		perror("read");
		return -1;
	}
	close(tray);
	return count;
}
int get_running_processes(struct running_processes *proc){
	//setup
	int status = 0;
	proc->count = 0;
	proc->executable_path = NULL; //realloc will allocate this

	//connect and handshake
	int tray = connect_tray_socket();
	if (tray < 0){
		fprintf(stderr,"Could not connect to tray socket.\n");
		status = -1;
		goto end;
	}
	int result = perform_handshake(tray,GET_PROCESSES);
	if (result < 0){
		fprintf(stderr,"could not handshake.\n");
		goto end;
	}
	
	//get the count
	int running_count;
	result = read(tray,&running_count,sizeof(int));
	if (result < 0){
		fprintf(stderr, "Could not get count.\n");
		perror("read");
		status = -1;
		goto end;
	}
	proc->count = running_count;

	proc->executable_path = (char**)malloc(sizeof(char*)*proc->count);
	for (int i = 0; i < running_count; i++){
		//allocate string size
		size_t buffer_size;
		result = read(tray,&buffer_size,sizeof(size_t));
		if (result < 0){
			fprintf(stderr,"could not read string size.\n");
			perror("read");
			status = -1;
			goto end;
		}
		proc->executable_path[i] = (char*)malloc(sizeof(char)*buffer_size);

		//get string
		result = read(tray,proc->executable_path[i],sizeof(char)*buffer_size);
		if (result < 0){
			status = -1;
			fprintf(stderr,"could not read string.\n");
			perror("read");
			goto end;
		}
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
	int result = perform_handshake(tray,NONE);
	if (result < 0){
		return -1;
	}
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
	return 1;
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
	
	int result = read(client, &request, sizeof(struct handshake));
	if (result < 0){
		fprintf(stderr,"tray: could not read from socket.\n");
		perror("read");
		goto client_cleanup;
	}
	//client version compatibility
	if (request.version != TRAY_VERSION){
		respond_handshake(client,BAD_VERSION);
	}

	switch (request.opcode){
		case NONE: //send an ok and do nothing
			respond_handshake(client,BAD_VERSION);
			goto client_cleanup;
		case KILL: //kill the tray
			respond_handshake(client,OK);
			return 1;
		case NEW_PROCESS:
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
				fprintf(stderr,"could not read command.\n");
				perror("read");
			}

			//fork
			pid_t pid = fork();
			if (pid < 0) perror("fork");
			if (pid == 0){
				result = system(command);
			}else{
				proc.count++;
				proc.pid = realloc(proc.pid,sizeof(pid_t)*proc.count);
				proc.pid[proc.count-1] = pid;
				proc.executable_path = realloc(proc.executable_path, sizeof(char *)*proc.count);
				proc.executable_path[proc.count-1] = malloc(sizeof(char) * command_size);
				snprintf(proc.executable_path[proc.count-1],command_size,"%s",command);
			}
			//clean
			free(command);
			break;
		case GET_PROCESS_COUNT:
			result = write(client,&proc.count,sizeof(int));
			if (result < 0){
				fprintf(stderr,"could not send process count.\n");
				perror("write");
			}
			break;
		case GET_PROCESSES:
			int count = proc.count;
			result = write(client,&count,sizeof(int));
			for (int i = 0; i < proc.count; i++){ //transmit all the strings
				size_t buffer_size = strlen(proc.executable_path[i])+1;
				result = write(client,&buffer_size,sizeof(size_t));
				if (result < 0){
					fprintf(stderr,"could not write the string length.\n");
					perror("write");
					goto client_cleanup;
				}
				result = write(client,proc.executable_path[i],sizeof(char)*buffer_size);
				if (result < 0){
					fprintf(stderr,"could not write string.\n");
					perror("write");
					goto client_cleanup;
				}
			}
			goto client_cleanup; //skip the response
	}

	client_cleanup:
	close(client);
	return 0;
}
