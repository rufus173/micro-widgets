#include <stdio.h>
#include <QFont>
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
#include <QGraphicsOpacityEffect>
#include <QScreen>
#include <QGridLayout>
#include <QWidget>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QListWidget>
#include <QScreen>
#include <QTimer>
#include <QKeyEvent>


//mine
#include "main.h"
#include "debug.h"
extern "C" {
#include "tray.h"
#include "applications.h"
}

//definitions
#define APPLICATION_HINT_COUNT 8
#define WINDOW_HEIGHT 140
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
void text_edited(QLineEdit *entry,QStandardItem **hints_item_list);

bool CustomLineEdit::event(QEvent *event){
	if (event->type() == QEvent::KeyPress){
		//casting tomfoolery
		QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
		if (key_event->key() == Qt::Key_Escape){
			printf("escaperlicious\n");
			emit escapePressed();
			//signify we have taken care of the signal
			return true;
		}else{
			//let some other handler deal with this
			return QWidget::event(event);
		}
		return true;
	}
	//let the signal pass through if we dont care about it
	return QWidget::event(event);
}

int main(int argc, char **argv){
	//====== load .desktop files ======
	app_list_head = get_all_applications();
	//so that the hints are in alphabetical order
	app_list_insertion_sort(app_list_head);

	//====== gui ======
	debug_class debug = debug_class("main");
	// ---------------------- GUI --------------------
	debug << "starting";
	//transparency effect
	QGraphicsOpacityEffect *hints_transparency_effect = new QGraphicsOpacityEffect();
	hints_transparency_effect->setOpacity(0.9);
	QGraphicsOpacityEffect *entry_transparency_effect = new QGraphicsOpacityEffect();
	entry_transparency_effect->setOpacity(0.9);

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
	main_window->setAttribute(Qt::WA_TranslucentBackground);

	//hit the grid(dy) 
	QBoxLayout *widget_grid = new QBoxLayout(QBoxLayout::LeftToRight,main_window);
	//widget_grid->setGeometry(0,0,ENTRY_WIDTH,WINDOW_HEIGHT);

	//entry box
	CustomLineEdit *entry = new CustomLineEdit();
	entry->setTextMargins(30,0,30,0);//left top right bottom
	QObject::connect(entry,&CustomLineEdit::escapePressed,&app,QApplication::closeAllWindows);

	//double font size
	QFont entry_font = entry->font();
	entry_font.setPointSize(entry_font.pointSize()*2);
	entry->setFont(entry_font);
	//make it able to expand verticaly
	entry->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	entry->setGraphicsEffect(entry_transparency_effect);
	widget_grid->addWidget(entry,0);

	//hints list
	QListView *hints_list = new QListView();
	hints_list->setGraphicsEffect(hints_transparency_effect);
	QStandardItemModel *hints_list_model;
	hints_list_model = new QStandardItemModel(3,1);
	hints_list->setModel(hints_list_model);
	hints_list->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	hints_list->setResizeMode(QListView::Adjust);
	QStandardItem *hints_item_list[APPLICATION_HINT_COUNT];
	for (int i = 0; i < APPLICATION_HINT_COUNT; i++){
		//memory handled by the QStandardModel
		QStandardItem *item = new QStandardItem();
		item->setSelectable(false);
		item->setEditable(false);
		hints_item_list[i] = item;
		hints_list_model->setItem(i,0,item);
	}

	widget_grid->addWidget(hints_list,0);
	//hints_list->setGeometry(0,0,);

	//link enter key to respective function
	QObject::connect(entry,&QLineEdit::returnPressed,[=]{enter_pressed(entry,main_window);});
	//link editing the text to updating the hints list
	QObject::connect(entry,&QLineEdit::textEdited,[=]{text_edited(entry,(QStandardItem**)hints_item_list);});

	//display everything
	debug << "running app.\n";
	main_window->show();
	int window_return = app.exec();
	
	//destroy everything
	delete main_window;
	delete hints_list_model;
	//delete hints_transparency_effect;
	//delete entry_transparency_effect;
	delete active_display;

	//====== run command ======
	if (entered_text != NULL) run_command(app_list_head,entered_text);

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
void text_edited(QLineEdit *entry,QStandardItem **hints_item_list){
	//====== extract current text from entry ======
	QByteArray byte_array = entry->text().toLatin1();
	entered_text = (char*)malloc(sizeof(char)*(byte_array.size()+1));
	strncpy(entered_text,byte_array.data(),byte_array.size()+1);

	//====== perform lookup ======
	struct application app_buffer[APPLICATION_HINT_COUNT];
	size_t app_buffer_length = APPLICATION_HINT_COUNT;
	
	//skip if the entered text is empty
	if (strlen(entered_text) == 0) app_buffer_length = 0;

	app_buffer_length = get_matching_applications(app_list_head,app_buffer,app_buffer_length,entered_text,0);
	size_t i;
	for (i = 0; i < app_buffer_length; i++){
		//printf("%s\n",app_buffer[i].name);
		hints_item_list[i]->setText(app_buffer[i].name);
		hints_item_list[i]->setEnabled(true);
	}
	for (; i < APPLICATION_HINT_COUNT; i++){
		hints_item_list[i]->setText("");
		hints_item_list[i]->setEnabled(false);
	}

	//====== cleanup ======
	free(entered_text);
}
