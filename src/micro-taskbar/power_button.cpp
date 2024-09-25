//standard
#include <stdio.h>
#include <unistd.h>

//meeeeee
#include <debug.h>

//QQQQQQQ
#include <QApplication>
#include <QGridLayout>
#include <QPushButton>

static debug_class debug("power_button");

void build_power_button(QGridLayout *master_grid, int column){
	debug << "building power button...";
	QGridLayout *widget_grid = new QGridLayout();

	//setup buttons
	QPushButton *shutdown_button = new QPushButton("Shutdown");
	QPushButton *restart_button = new QPushButton("Restart");
	shutdown_button->setFixedWidth(100);
	restart_button->setFixedWidth(100);
	widget_grid->addWidget(shutdown_button,0,0,1,1);
	widget_grid->addWidget(restart_button,1,0,1,1);

	master_grid->addLayout(widget_grid,0,column,1,1);
	debug << "power button built";
}
