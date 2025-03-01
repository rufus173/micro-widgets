#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <setjmp.h>

//QT
#include <QApplication>
#include <QSizePolicy>
#include <QLabel>
#include <QWindow>
#include <QScreen>
#include <QGridLayout>
#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QScreen>
#include <QTimer>


//mine
#include "debug.h"
extern "C" {
#include "tray.h"
#include "applications.h"
}

//definitions
#define WINDOW_HEIGHT 100
#define ENTRY_WIDTH 200
#define HINTS_WIDTH 200
#define WINDOW_WIDTH ENTRY_WIDTH + HINTS_WIDTH

//globals
int active_display_height;
int active_display_width;
int left_x_when_centred;
char *entered_text = NULL;

void move_window_step(QTimer *move_loop,QWidget *window);
void enter_pressed(QLineEdit *entry, QWidget *window);

int main(int argc, char **argv){
	//====== load .desktop files ======
	struct applications_head *applications_list_head = get_all_applications();
	free_applications(applications_list_head);

	//====== gui ======
	debug_class debug = debug_class("main");
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

	//hit the grid(dy) 
	QBoxLayout *widget_grid = new QBoxLayout(QBoxLayout::LeftToRight,main_window);
	//widget_grid->setGeometry(0,0,ENTRY_WIDTH,WINDOW_HEIGHT);

	//entry box
	QLineEdit *entry = new QLineEdit();
	entry->setTextMargins(10,0,10,0);//left top right bottom
	//make it able to expand verticaly
	entry->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	widget_grid->addWidget(entry,1);

	//suggestions list
	QListWidget *suggestions_list = new QListWidget();
	suggestions_list->addItem("hi");
	suggestions_list->addItem("bye");
	suggestions_list->addItem(":)");
	widget_grid->addWidget(suggestions_list,1);
	//suggestions_list->setGeometry(0,0,);

	//link enter key to respective function
	QObject::connect(entry,&QLineEdit::returnPressed,[=]{enter_pressed(entry,main_window);});

	//display everything
	debug << "running app.\n";
	main_window->show();
	int window_return = app.exec();
	
	//destroy everything
	delete main_window;
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
		entered_text = (char*)malloc(sizeof(char)*(byte_array.size()+1));
		strncpy(entered_text,byte_array.data(),byte_array.size()+1);
	}else{
		entered_text = NULL;
	}
	window->close();
}
