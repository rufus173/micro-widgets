#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <stdio.h>

class debug{
	private:
		char function_name[1024] = "main";
	public:
		debug(const char *name){
			snprintf(function_name,1024,"%s",name);
		}
		void write_stderr(char *buffer){
			fprintf(stderr,"ERROR in [%s]: %s\n",function_name,buffer);
		}
		void write_stdout(char *buffer){
			printf("DEBUG: %s\n",buffer);
		}
		void operator<<(const char *buffer){
			write_stdout((char *)buffer);
		}
		void operator<(const char *buffer){
			write_stderr((char *)buffer);
		}
};
static debug debug("main");

void b1p();
void b2p();

int main(int argc, char **argv){
	QApplication app = QApplication(argc,argv);
	QWidget *window = new QWidget();

	QPushButton *button1 = new QPushButton("app1",window);
	QPushButton *button2 = new QPushButton("app2",window);
	QObject::connect(button1,&QPushButton::clicked,b1p);
	QObject::connect(button2,&QPushButton::clicked,b2p);

	QGridLayout *grid = new QGridLayout(window);
	grid->addWidget(button1,0,0,1,2);
	grid->addWidget(button2,0,2,1,1);

	window->show();
	int status = app.exec();
	return status;
}
void b1p(){
	debug << "i am alive";
}
void b2p(){
	debug < "i am dead";
}
