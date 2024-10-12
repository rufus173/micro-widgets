#include <stdio.h>

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

//definitions
#define WINDOW_WIDTH 700
#define ENTRY_HEIGHT 100
#define HINT_MENU_HEIGHT 0

//globals
debug_class debug = debug_class("main");
int active_display_height;
int active_display_width;
int left_x_when_centred;

void move_window_step(QTimer *move_loop,QWidget *window);
int main(int argc, char **argv){
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

	//slide the window down with some recursive nonsense
	QTimer *move_loop = new QTimer();
	QObject::connect(move_loop,&QTimer::timeout,[=]{move_window_step(move_loop,main_window);});
	move_loop->start(10);

	//link enter key to respective function

	//display everything
	debug << "running app.\n";
	main_window->show();
	return app.exec();
}
void enter_pressed(QWidget *entry){
	;
}
void move_window_step(QTimer *move_loop,QWidget *window){
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
