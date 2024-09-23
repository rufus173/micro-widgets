//standard headers
#include <stdio.h>

//my headers
#include "debug.h"
#include "config.h"

//qt headers
#include <QApplication>
#include <QScreen>
#include <QGridLayout>
#include <QPushButton>
#include <QTimer>

//function prototypes


//global variables
static class debug_class debug("main");//initialise debug

int main(int argc, char **argv){
	debug << "Started";

	//variable definitions
	int status = 0;
	char buffer[1024];
	struct bar_dimentions bar_dimentions;
	struct screen_info screen_info;
	
	QApplication app(argc,argv);
	QWidget *window = new QWidget();
	window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);//make window frameless
	//get screen info
	QScreen *screen = QGuiApplication::primaryScreen();
	screen_info.height = screen->geometry().height();
	screen_info.width = screen->geometry().width();
	snprintf(buffer,1024,"Screen size %dx%d",screen_info.width,screen_info.height);
	debug << (const char *)buffer;
	//set window size
	bar_dimentions.height = 50;
	bar_dimentions.width = screen_info.width;
	bar_dimentions.x = 0;
	bar_dimentions.y = screen_info.height-bar_dimentions.height;
	window->setGeometry(bar_dimentions.x,bar_dimentions.y,bar_dimentions.width,bar_dimentions.height);

	window->show();
	status = app.exec();

	debug << "Finnished";
	return status;	
}
