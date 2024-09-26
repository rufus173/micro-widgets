//standard headers
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

//my headers
#include "debug.h"
#include "config.h"
#include "modules.h"

//qt headers
#include <QApplication>
#include <QScreen>
#include <QFile>
#include <QGridLayout>
#include <QPushButton>
#include <QTimer>

//function prototypes
QGridLayout *build_grid(QWidget *window);

//global variables
static class debug_class debug("main");//initialise debug

int main(int argc, char **argv){
	debug << "Started";

	//variable definitions
	int status = 0;
	char buffer[1024];
	char config_directory[1024];
	char *user_home = getenv("HOME");
	char style_sheet_file_path[1024];
	struct bar_dimentions bar_dimentions;
	struct screen_info screen_info;
	if (user_home == NULL){
		debug < "could not get $HOME, resorting to getpwuid";
		user_home = getpwuid(getuid())->pw_dir;
	}
	int result = snprintf(config_directory,1024,"%s/.config/micro-taskbar",user_home);
	result = snprintf(style_sheet_file_path,1024,"%s/style.qss",config_directory);
	if (result < 0){
		debug < "style sheet path to long";
	}
	
	QApplication app(argc,argv);
	QWidget *window = new QWidget();
	window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);//make window frameless
	//set style sheet
	if (access(style_sheet_file_path,R_OK) == 0){
		QFile style_sheet_file(style_sheet_file_path);
		style_sheet_file.open(QFile::ReadOnly);
		app.setStyleSheet(style_sheet_file.readAll());
	}else{
		debug < "could not access style sheet";
	}

	//get screen info
	QScreen *screen = QGuiApplication::primaryScreen();
	screen_info.height = screen->geometry().height();
	screen_info.width = screen->geometry().width();
	snprintf(buffer,1024,"Screen size %dx%d",screen_info.width,screen_info.height);
	debug << (const char *)buffer;
	//set window size
	bar_dimentions.height = 70;
	bar_dimentions.width = screen_info.width;
	bar_dimentions.x = 0;
	bar_dimentions.y = screen_info.height-bar_dimentions.height;
	window->setGeometry(bar_dimentions.x,bar_dimentions.y,bar_dimentions.width,bar_dimentions.height);

	//build the taskbar
	// main grid[[widget a grid],[widget b grid],[widget c grid]]
	QGridLayout *grid = build_grid(window);
	build_battery_level(grid,0);
	build_digital_clock(grid,1/*column 1*/);
	//mid section free to allow expantion of taskbar
	build_quick_launcher(grid,3);
	build_power_button(grid,4);

	window->show();
	status = app.exec();

	debug << "Finnished";
	//clean up
	return status;	
}
QGridLayout *build_grid(QWidget *window){
	QGridLayout *grid = new QGridLayout(window);
	return grid;
}
