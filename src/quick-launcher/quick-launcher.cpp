#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

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
	
	//setup grid
	QGridLayout *grid = new QGridLayout(window);

	//setup widget storage
	QPushButton **launch_button_storage = (QPushButton**)malloc(sizeof(QPushButton*)*programs->count());

	//proceduraly generate buttons for each app
	for (int i = 0; i < programs->count(); i++){
		printf("%d\n",i);

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

	//present to user
	window->show();
	status = app.exec();
	
	//cleanup
	delete programs;
	return status;
}
int load_programs(){
	int status = 0;
	programs->add("firefox","/usr/bin/firefox");
	programs->add("xterm","/usr/bin/xterm");
	printf("loaded %d programs\n",programs->count());
	return status;
}
void launch_button_pressed(int index){
	debug << (const char *)programs->get_launch_command(index);
}
