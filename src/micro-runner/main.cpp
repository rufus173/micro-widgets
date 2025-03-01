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
#include <QStandardItemModel>
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
#define WINDOW_HEIGHT 120
#define ENTRY_WIDTH 400
#define HINTS_WIDTH 300
#define WINDOW_WIDTH ENTRY_WIDTH + HINTS_WIDTH

//globals
int active_display_height;
int active_display_width;
int left_x_when_centred;
char *entered_text = NULL;
struct applications_head *app_list_head;

void enter_pressed(QLineEdit *entry, QWidget *window);
void text_edited(QLineEdit *entry,QStandardItemModel *hints_list_model);

int main(int argc, char **argv){
	//====== load .desktop files ======
	app_list_head = get_all_applications();

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
	main_window->setFixedSize(WINDOW_WIDTH,WINDOW_HEIGHT);

	//hit the grid(dy) 
	QBoxLayout *widget_grid = new QBoxLayout(QBoxLayout::LeftToRight,main_window);
	//widget_grid->setGeometry(0,0,ENTRY_WIDTH,WINDOW_HEIGHT);

	//entry box
	QLineEdit *entry = new QLineEdit();
	entry->setTextMargins(10,0,10,0);//left top right bottom
	//make it able to expand verticaly
	entry->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	widget_grid->addWidget(entry,0);

	//hints list
	QListView *hints_list = new QListView();
	QStandardItemModel *hints_list_model;
	hints_list_model = new QStandardItemModel(3,1);
	hints_list->setModel(hints_list_model);
	hints_list->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	hints_list->setResizeMode(QListView::Adjust);

	widget_grid->addWidget(hints_list,0);
	//hints_list->setGeometry(0,0,);

	//link enter key to respective function
	QObject::connect(entry,&QLineEdit::returnPressed,[=]{enter_pressed(entry,main_window);});
	//link editing the text to updating the hints list
	QObject::connect(entry,&QLineEdit::textEdited,[=]{text_edited(entry,hints_list_model);});

	//display everything
	debug << "running app.\n";
	main_window->show();
	int window_return = app.exec();
	
	//destroy everything
	delete main_window;
	delete hints_list_model;
	delete active_display;

	//====== unload .desktop files ======
	free_applications(app_list_head);

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
void text_edited(QLineEdit *entry,QStandardItemModel *hints_list_model){
	//printf("text changed\n");
}
