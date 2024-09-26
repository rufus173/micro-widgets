//header yourself
#include <QGridLayout>
//header myself
#include "debug.h"

static debug_class debug = debug_class("quick_launcher");

void build_quick_launcher(QGridLayout *master_grid, int column){
	debug << "starting quick launcher construction";
	QGridLayout *widget_grid = new QGridLayout();

	//procedural button creation
	//i cant be bothered rn do this later

	master_grid->addLayout(widget_grid,0,column,1,1);
}
