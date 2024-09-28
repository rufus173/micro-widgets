//header qt
#include <QGridLayout>
#include <QPushButton>
#include <QApplication>
//header me
#include "debug.h"
extern "C" {
#include "tray.h"
}
//standard
#include <stdlib.h>

static debug_class debug = debug_class("quick_launcher");

//my classes
class class_quick_applications {
	private:
	struct applications {
		int count = 0;
		char display_name[10][1024];
		char executable_location[10][1024];
	};
	struct applications apps;
	public:
	//class_quick_application(){
	//	;
	//}
	void add(const char *display_name, const char *executable_location){
		apps.count++;
		if (apps.count > 10){
			debug < "too many apps";
			apps.count--;//reset to original value
			return;
		}

		//copy over strings
		int result;
		result = snprintf(apps.display_name[apps.count-1],1024,"%s",display_name);
		if (result > 1024) debug < "display name truncated";
		result = snprintf(apps.executable_location[apps.count-1],1024,"%s",executable_location);
		if (result > 1024) debug < "executable path truncated";
	}
	int count(){return apps.count;}
	char *display_name(int index){return apps.display_name[index];}
	char *executable_location(int index){return apps.executable_location[index];}
};

void build_quick_launcher(QGridLayout *master_grid, int column){
	debug << "starting quick launcher construction";
	QGridLayout *widget_grid = new QGridLayout();

	//add some apps
	static class_quick_applications apps = class_quick_applications();
	apps.add("firefox","/usr/bin/firefox");
	apps.add("xterm","/usr/bin/xterm");

	//procedural button creation
	//set up an array
	QPushButton *launch_button_array[10]; //max 10 buttons (i cant be bothered to dynamicaly allocate and deallocate)
	for (int i = 0; i < apps.count(); i++){
		char output_buffer[1024];
		snprintf(output_buffer,1024,"adding button for %s",apps.display_name(i));
		debug << (const char *)output_buffer;
		
		//creating the buttons
		launch_button_array[i] = new QPushButton(apps.display_name(i));
		char *executable_location = apps.executable_location(i);
		QObject::connect(launch_button_array[i],&QPushButton::clicked,[i,executable_location]{
			char output_buffer[1024];
			snprintf(output_buffer,1024,"launching %s",executable_location);
			debug << output_buffer;
			start_program(executable_location);
		});
		launch_button_array[i]->setFixedWidth(80);
		widget_grid->addWidget(launch_button_array[i],0,i,1,1);
	}

	master_grid->addLayout(widget_grid,0,column,1,1);
	debug << "quick launcher done";
}
