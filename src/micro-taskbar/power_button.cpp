//standard
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

//meeeeee
#include <debug.h>

//QQQQQQQ
#include <QApplication>
#include <QGridLayout>
#include <QPushButton>

#define SHUTDOWN_COMMAND "shutdown now"
#define RESTART_COMMAND "reboot"

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

	//button functions
	//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
	QObject::connect(shutdown_button,&QPushButton::clicked, []{
		debug << "shutdown button pressed";
		int status = system(SHUTDOWN_COMMAND);
		printf("shutdown status: %d\n",status);
	});
	QObject::connect(restart_button,&QPushButton::clicked, []{
		debug << "restart button pressed";
		int status = system(RESTART_COMMAND);
		printf("restart status: %d\n",status);
	});

	master_grid->addLayout(widget_grid,0,column,1,1);
	debug << "power button built";
}
