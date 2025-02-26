#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <setjmp.h>

//QT
#include <QApplication>
#include <QWindow>
#include <QScreen>
#include <QWidget>
#include <QLineEdit>
#include <QScreen>
#include <QTimer>

//mine
#include "debug.h"
extern "C" {
#include "tray.h"
#include "applications.h"
}

//definitions
#define WINDOW_WIDTH 700
#define ENTRY_HEIGHT 100
#define HINT_MENU_HEIGHT 0

//globals
int active_display_height;
int active_display_width;
int left_x_when_centred;
char *command = NULL;

int build_gui(int argc,char **argv);
void move_window_step(QTimer *move_loop,QWidget *window);
void enter_pressed(QLineEdit *entry, QWidget *window);

int main(int argc, char **argv){
	//load .desktop files
	get_all_applications();
	//gui
	build_gui(argc,argv);//causes exit() to hang
	// ---------------------------- command execution ----------------------
	printf("window closed\n");
	if (command == NULL){
		printf("no command given\n");
		return 0;
	}
	
	printf("ready to execute %s\n", command);
	pid_t child_pid = -1;
	if (check_tray_status() < 0){
		printf("tray offline, forking one now.\n");
		child_pid = fork();
		if (child_pid < 0){
			fprintf(stderr,"ERROR: Could not fork. aborting\n");
			perror("fork");
			return 1;
		}
	}else{
		 printf("Tray ready to communicate with\n");
	}
	// ---------------- child ------------
	if (child_pid == 0){
		//change the process short name
		prctl(PR_SET_NAME, /*idk what this is>*/(unsigned long)"tray", 0, 0, 0);
		printf("child starting tray\n");
		main_tray(TRAY_NO_PERSIST); //close after last program closes
		printf("child tray fork exiting\n");
		abort();//exit gets stuck
	}
	// ---------------- parent -----------
	sleep(1); //give the tray time
	printf("sending command to tray\n");
	int result = start_program(command);
	free(command);
	printf("tray request result: %d\n",result);
	printf("waiting for tray fork on pid %d to close\n",child_pid);
	result = waitpid(child_pid,NULL,WUNTRACED);
	if (result < 0){
		fprintf(stderr,"could not wait for tray.\n");
		perror("waitpid");
		return 1;
	}
	return 0;
}
int build_gui(int argc, char **argv){
	debug_class debug = debug_class("build_gui");
	// ---------------------- GUI --------------------
	debug << "starting";
	//create the app
	QApplication app = QApplication(argc,argv);

	//get info about the screen
	QScreen *active_display = QGuiApplication::primaryScreen();
	active_display_height = active_display->geometry().height();
	active_display_width = active_display->geometry().width();
	left_x_when_centred = (active_display_width/2)-(WINDOW_WIDTH/2);
	
	//setup window
	QWidget *main_window = new QWidget();
	main_window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

	//entry box
	QLineEdit *entry = new QLineEdit(main_window);
	entry->setGeometry(0,0,WINDOW_WIDTH,ENTRY_HEIGHT);
	entry->setTextMargins(20,0,20,0);//left top right bottom

	//slide the window down with some recursive nonsense
	QTimer *move_loop = new QTimer();
	QObject::connect(move_loop,&QTimer::timeout,[=]{move_window_step(move_loop,main_window);});
	move_loop->start(10);

	//link enter key to respective function
	QObject::connect(entry,&QLineEdit::returnPressed,[=]{enter_pressed(entry,main_window);});

	//display everything
	debug << "running app.\n";
	main_window->show();
	int window_return = app.exec();
	
	//destroy everything
	delete main_window;
	delete move_loop;
	delete active_display;

	if (window_return != 0){
		debug < "window failed";
		return window_return;
	}
	return 0;
}
void enter_pressed(QLineEdit *entry, QWidget *window){
	if (entry->text().isEmpty() != true){
		QByteArray byte_array = entry->text().toLatin1();
		command = (char*)malloc(sizeof(char)*(byte_array.size()+1));
		strncpy(command,byte_array.data(),byte_array.size()+1);
	}else{
		command = NULL;
	}
	window->close();
}
void move_window_step(QTimer *move_loop,QWidget *window){
	debug_class debug = debug_class("move_window_step");
	//the point when it slows down
	int threshold = 50;

	static int y_pos = 0;
	window->move(left_x_when_centred,y_pos);
	//slow down when we get close to the centre
	int target_y = (active_display_height/2)-(ENTRY_HEIGHT+HINT_MENU_HEIGHT);
	int offset = 0;
	if (y_pos > target_y - threshold){
		double scaler = (float)(target_y-y_pos)/(float)threshold;
		offset = 10 - (9 * scaler * scaler);
	}
	y_pos += 10-offset;
	
	//stop when done
	if (y_pos > target_y){
		window->move(left_x_when_centred,target_y);
		move_loop->stop();
		debug << "final window position reached";
	}
}
