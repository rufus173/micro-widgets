#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <QTimer>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_CHILD_PROCESSES 10

class Debug_class{
	private:
		char function_name[1024] = "main";
	public:
		Debug_class(const char *name){
			snprintf(function_name,1024,"%s",name);
		}
		void write_stderr(char *function_name, char *buffer){
			fprintf(stderr,"ERROR in [%s]: %s\n",function_name, buffer);
		}
		void write_stdout(char *function_name, char *buffer){
			printf("DEBUG in [%s]: %s\n",function_name, buffer);
		}
		void operator<<(const char *buffer){
			write_stdout(function_name, (char *)buffer);
		}
		void operator<(const char *buffer){
			write_stderr(function_name, (char *)buffer);
		}
};
class Programs{
private:
	struct program_info{
		char **app_name;
		char **app_launch_command;
	};
	struct program_storage{
		int program_count = 0;
		struct program_info program_info;
	};
	struct program_storage program_storage;
public:
	Programs(){
		program_storage.program_count = 0;
		program_storage.program_info.app_name = (char**)malloc(sizeof(char*));
		program_storage.program_info.app_launch_command = (char**)malloc(sizeof(char*));
	}
	int add(const char *display_name, const char *executable_location){
		int status = 0;
		if (access(executable_location,F_OK) != 0){
			return -1;
		}
		program_storage.program_count += 1;
		program_storage.program_info.app_name[program_storage.program_count-1] = (char *)malloc(sizeof(char)*(strlen(display_name)+1));
		strncpy(program_storage.program_info.app_name[program_storage.program_count-1],display_name,strlen(display_name)+1);

		program_storage.program_info.app_launch_command[program_storage.program_count-1] = (char *)malloc(sizeof(char)*(strlen(executable_location)+1));
		strncpy(program_storage.program_info.app_launch_command[program_storage.program_count-1],executable_location,strlen(executable_location)+1);
		return status;
	}
	int count(){
		return program_storage.program_count;
	}
	char *get_display_name(int index){
		return program_storage.program_info.app_name[index];
	}
	char *get_launch_command(int index){
		return program_storage.program_info.app_launch_command[index];
	}
};
struct child_processes_struct{
	int count = 0;
	pid_t pid[MAX_CHILD_PROCESSES];
};
volatile struct child_processes_struct child_processes;

static Debug_class debug("main");
static Programs *programs = new Programs();

void launch_button_pressed(int index);
int load_programs();

int main(int argc, char **argv){
	//load up the programs pisplayed in the menu
	int status = load_programs();
	if (status != 0) return status;

	//set up the main app and window
	QApplication app = QApplication(argc,argv);
	QWidget *window = new QWidget();
	window->setWindowTitle("quick launcher");
	
	//setup grid
	QGridLayout *grid = new QGridLayout(window);

	//setup widget storage
	QPushButton **launch_button_storage = (QPushButton**)malloc(sizeof(QPushButton*)*programs->count());

	//proceduraly generate buttons for each app
	for (int i = 0; i < programs->count(); i++){

		//set up all the buttons
		launch_button_storage[i] = new QPushButton((const char *)programs->get_display_name(i),window);
		QObject::connect(launch_button_storage[i],&QPushButton::clicked,
			[i](){//capture i by value
				launch_button_pressed(i);
			}
		);

		//add everything to a grid
		grid->addWidget(launch_button_storage[i],0,i,1,1);
	}

	//setup a mainloop to wait for finnished child processes
	QTimer *timer = new QTimer();
	QObject::connect(timer, &QTimer::timeout, []{
			for (int i = 0; i < child_processes.count; i++){
				int status = waitpid(child_processes.pid[i],NULL,WNOHANG);//dont hang if no children are finnished
				if (status > 0){
					debug << "collected process";
					child_processes.pid[i] = child_processes.pid[child_processes.count-1];
					child_processes.count--;
					printf("collected: %d,remaining processes: %d\n",child_processes.pid[i],child_processes.count);
					break;// we changed the struct size so dont keep itterating
				}

			}
		});
	timer->start(500);

	//present to user
	window->show();
	status = app.exec();
	
	//cleanup
	delete programs;
	//wait for child processes to finnish
	for (int i = 0; i < child_processes.count; i++){
		char output_buffer[1024];
		snprintf(output_buffer,1024,"waiting on %d, %d processes remaining",child_processes.pid[i],child_processes.count-i);
		debug << (const char*)output_buffer;
		int status = waitpid(child_processes.pid[child_processes.count],NULL,0);
		//printf("wait returned %d\n",status);
		if (status < 0){
			debug < "could not wait for process";
			perror("waitpid");
		}
	}
	return status;
}
int load_programs(){
	int status = 0;
	programs->add("firefox","/usr/bin/firefox");
	programs->add("xterm","/usr/bin/xterm");
	char output_buffer[1024];
	snprintf(output_buffer,1024,"loaded %d programs",programs->count());
	debug << (const char *)output_buffer;
	return status;
}
void launch_button_pressed(int index){
	debug << (const char *)programs->get_launch_command(index);

	//max 10 processes
	if (child_processes.count == 10){
		debug < "Max 10 processes at one time";
		return;
	}

	//start another process to manage
	pid_t pid = fork();
	if (pid < 0){
		debug < "error occured when trying to fork";
		perror("fork");
		return;
	}
	child_processes.pid[child_processes.count] = pid;
	child_processes.count++;
	if (pid == 0){
		//child process
		int result = system((const char*)programs->get_launch_command(index));
		if (result < 0){
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);

	}else{
		//parent process
	}
}
