#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <stdio.h>

class debug_class{
	private:
		char function_name[1024] = "main";
	public:
		debug_class(const char *name){
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
class programs{
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
	programs(){
		program_storage.program_count = 0;
	}
};

static debug_class debug("main");

void app_1();
void app_2();

int main(int argc, char **argv){
	//set up the main app and window
	QApplication app = QApplication(argc,argv);
	QWidget *window = new QWidget();

	//set up all the buttons
	QPushButton *app_1_button = new QPushButton("app1",window);
	QPushButton *app_2_button = new QPushButton("app2",window);
	QObject::connect(app_1_button,&QPushButton::clicked,app_1);
	QObject::connect(app_2_button,&QPushButton::clicked,app_2);

	//add everything to a grid
	QGridLayout *grid = new QGridLayout(window);
	grid->addWidget(app_1_button,0,0,1,1);
	grid->addWidget(app_2_button,0,1,1,1);

	//present to user
	window->show();
	int status = app.exec();
	return status;
}
void app_1(){
	debug << "i am alive";
}
void app_2(){
	debug < "i am dead";
}
