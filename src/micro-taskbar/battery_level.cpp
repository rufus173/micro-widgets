//my headers
#include "debug.h"

//gtk headers
#include <QGridLayout>
#include <QApplication>

//standar headers

static debug_class debug("battery_level");

void build_battery_level(QGridLayout *master_grid,int column){
	debug << "building battery level widget";
	QGridLayout *battery_level_grid = new QGridLayout();
	debug << "adding to grid";
	master_grid->addLayout(battery_level_grid,0,column,1,1);
	debug << "done";
}
