//header qt
#include <QGridLayout>
#include <QPushButton>
#include <QApplication>
#include <QTimer>
#include <QLabel>
//header me
#include "debug.h"
extern "C" {
#include "tray.h"
}
//standard
#include <stdlib.h>
#include <string.h>

static debug_class debug = debug_class("quick_launcher");

//my classes
class class_quick_applications {
	private:
	struct applications {
		int count = 0;
		char display_name[10][1024];
		char executable_location[10][1024];
	};
	struct app {
		int count = 0;
		char *executable_path;
		char *display_name;
	};
	struct tracked_applications {
		int count = 0;
		struct app *app;
	};

	struct applications apps;
	struct tracked_applications tracked_apps;
	int running_app_count = 0;
	public:
	class_quick_applications(){
		tracked_apps.app = (struct app*)malloc(sizeof(struct app));
	}
	~class_quick_applications(){
		free(tracked_apps.app);
	}
	void add(const char *display_name, const char *executable_location){
		apps.count++;
		if (apps.count > 10){
			debug < "too many apps";
			apps.count--;//reset to original value
			return;
		}
		//add it to the tracked apps struct for later
		tracked_apps.count++;
		tracked_apps.app = (struct app*)realloc(tracked_apps.app,sizeof(struct app)*tracked_apps.count);
		tracked_apps.app[tracked_apps.count-1].count = 0;
		tracked_apps.app[tracked_apps.count-1].display_name = (char*)malloc(sizeof(char)*(strlen(display_name)+1));
		tracked_apps.app[tracked_apps.count-1].executable_path = (char*)malloc(sizeof(char)*(strlen(executable_location)+1));
		memcpy(tracked_apps.app[tracked_apps.count-1].display_name,display_name,strlen(display_name)+1);
		memcpy(tracked_apps.app[tracked_apps.count-1].executable_path,executable_location,strlen(executable_location)+1);

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
	
	//tray interactions
	void update_running(){
		running_app_count = get_running_program_count();
		struct running_processes procs;
		get_running_processes(&procs);
		//find the matching app struct and increment its count counter
		for (int x = 0; x < tracked_apps.count; x++){
			tracked_apps.app[x].count = 0;
			for (int i = 0; i < procs.count; i++){
				if (strcmp(tracked_apps.app[x].executable_path,procs.executable_path[i]) == 0){
					tracked_apps.app[x].count++;
				}
			}
		}
		free_running_processes_struct(&procs);
	}
	int running_app_get_count(int index){
		return tracked_apps.app[index].count;
	}
	int running_count(){return running_app_count;}
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
	QLabel *status_label_array[10]; //displays how many of each app is open
	QTimer *status_label_update_loop_array[10];
	int i;
	for (i = 0; i < apps.count(); i++){
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
		widget_grid->addWidget(launch_button_array[i],1,i,2,1);

		//create status labels
		status_label_array[i] = new QLabel("0");
		status_label_array[i]->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
		widget_grid->addWidget(status_label_array[i],0,i,1,1);

		//status label update loop
		status_label_update_loop_array[i] = new QTimer();
		QObject::connect(status_label_update_loop_array[i],&QTimer::timeout,[i,status_label_array]{
			char buffer[1024];
			snprintf(buffer,1024,"%d",apps.running_app_get_count(i));
			status_label_array[i]->setText(buffer);
		});
		status_label_update_loop_array[i]->start(500);
	}

	//create the label for total programs open
	QLabel *total_open_programs = new QLabel("programs\nopen: 0");
	total_open_programs->setFixedWidth(80);
	widget_grid->addWidget(total_open_programs,0,i,3,1);
	QTimer *open_programs_update_loop = new QTimer();
	QObject::connect(open_programs_update_loop,&QTimer::timeout,[total_open_programs]{
		//for the display when an app is open
		apps.update_running();
		char text[1024];
		snprintf(text,1024,"programs\nopen: %d",apps.running_count());
		total_open_programs->setText(text);
	});
	open_programs_update_loop->start(500);

	master_grid->addLayout(widget_grid,0,column,1,1);
	debug << "quick launcher done";
}
