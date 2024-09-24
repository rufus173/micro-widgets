//my headers
#include "debug.h"

//standard headers
#include <time.h>

//qt headers
#include <QGridLayout>
#include <QTimer>
#include <QLabel>
#include <QApplication>

static debug_class debug("digital_clock");

static char *date_string();
static char *time_string();

void build_digital_clock(QGridLayout *master_grid, int column){
	debug << "building clock";
	//creating widgets
	QGridLayout *widget_grid = new QGridLayout();
	widget_grid->setAlignment(Qt::AlignLeft);
	QLabel *date = new QLabel("date");
	QLabel *time = new QLabel("time");
	//date->setFixedWidth(100);
	//time->setFixedWidth(100);

	//stack them all neatly
	widget_grid->addWidget(date,0,0,1,1);
	widget_grid->addWidget(time,1,0,1,1);
	master_grid->addLayout(widget_grid,0,column,1,1);

	//create a time to update them
	QTimer *digital_clock_update_timer = new QTimer();
	QObject::connect(digital_clock_update_timer, &QTimer::timeout, [date,time]{
			//update loop
			date->setText(date_string());
			time->setText(time_string());
		});
	debug << "starting timer";
	digital_clock_update_timer->start(100);
	debug << "done";
}
static char *date_string(){
	time_t time_now = time(NULL);
	static char return_buffer[1024];
	struct tm *time_struct;
	time_struct = localtime(&time_now);
	strftime(return_buffer,1024,"%d/%m/%y",time_struct);
	return return_buffer;
}	
static char *time_string(){
	time_t time_now = time(NULL);
	static char return_buffer[1024];
	struct tm *time_struct;
	time_struct = localtime(&time_now);
	strftime(return_buffer,1024,"%H:%M",time_struct);
	return return_buffer;
}	
