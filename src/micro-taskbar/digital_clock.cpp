//my headers
#include "debug.h"

//qt headers
#include <QGridLayout>
#include <QTimer>
#include <QLabel>
#include <QApplication>

static debug_class debug("digital_clock");

void build_digital_clock(QGridLayout *master_grid, int column){
	//creating widgets
	QGridLayout *widget_grid = new QGridLayout();
	QLabel *date = new QLabel("date");
	QLabel *time = new QLabel("time");

	//stack them all neatly
	widget_grid->addWidget(date,0,0,1,1);
	widget_grid->addWidget(time,1,0,1,1);
	master_grid->addLayout(widget_grid,0,column,1,1);
}
