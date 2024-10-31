//my headers
#include "debug.h"

//gtk headers
#include <QGridLayout>
#include <QApplication>
#include <QLabel>
#include <QProgressBar>
#include <QRect> 
#include <QTimer>

//standar headers
#include <unistd.h>

static int get_battery_percent(int battery);

static debug_class debug("battery_level");

void build_battery_level(QGridLayout *master_grid,int column){
	debug << "building battery level widget";

	//build the widgets
	//QLabel *battery_percent_label = new QLabel("Battery 100%");
	// ======== bat 1 =========
	QProgressBar *battery_1_percent_bar = new QProgressBar();
	battery_1_percent_bar->setRange(0,100);
	//battery_percent_bar->setTextVisible(true);
	battery_1_percent_bar->setOrientation(Qt::Vertical);
	battery_1_percent_bar->setAlignment(Qt::AlignLeft);
	battery_1_percent_bar->setFormat("%p%");
	battery_1_percent_bar->setFixedWidth(50);
	battery_1_percent_bar->setFixedHeight(50);
	// ========= bat 2 ========
	QProgressBar *battery_2_percent_bar = new QProgressBar();
	battery_2_percent_bar->setRange(0,100);
	//battery_percent_bar->setTextVisible(true);
	battery_2_percent_bar->setOrientation(Qt::Vertical);
	battery_2_percent_bar->setAlignment(Qt::AlignLeft);
	battery_2_percent_bar->setFormat("%p%");
	battery_2_percent_bar->setFixedWidth(50);
	battery_2_percent_bar->setFixedHeight(50);

	//create update loop
	QTimer *battery_level_update_timer = new QTimer();
	QObject::connect(battery_level_update_timer,&QTimer::timeout, [battery_1_percent_bar,battery_2_percent_bar]{
		battery_1_percent_bar->setValue(get_battery_percent(0));
		battery_2_percent_bar->setValue(get_battery_percent(1));
	});
	battery_level_update_timer->start(1000);

	//create a widget grid
	QGridLayout *widget_grid = new QGridLayout();
	widget_grid->addWidget(battery_1_percent_bar,0,0,1,1);
	widget_grid->addWidget(battery_2_percent_bar,0,1,1,1);
	//add to the master grid
	debug << "adding to grid";
	master_grid->addLayout(widget_grid,0,column,1,1);
	debug << "done";
}
static int get_battery_percent(int battery_number){
	static bool battery_available = true;
	if (!battery_available) return 100;

	char location_energy_now[1024];
	char location_energy_full[1024];
	snprintf(location_energy_now,sizeof(location_energy_full),"/sys/class/power_supply/BAT%d/energy_now",battery_number);
	snprintf(location_energy_full,sizeof(location_energy_full),"/sys/class/power_supply/BAT%d/energy_full",battery_number);
	//check if the battery is available
        if (access(location_energy_now,F_OK) != 0){
		debug < "could not access battery data";
		battery_available = false;
                return 100;
        }

        FILE *charge_now_fd = fopen(location_energy_now,"r");
        FILE *charge_full_fd = fopen(location_energy_full,"r");
        if (charge_now_fd == NULL || charge_full_fd == NULL){
		debug < "problems opening battery file descriptors";
                perror("fopen");
                return 100;
        }

        char buffer[1024];
        int buffer_size = fread(buffer,1,1024,charge_now_fd);
        if (buffer_size == 0){
                fprintf(stderr,"ERROR: Could not read file.\n");
                return -1;
        }
        int charge_now = strtol(buffer,NULL,10);

        buffer_size = fread(buffer,1,1024,charge_full_fd);
        if (buffer_size == 0){
                fprintf(stderr,"ERROR: Could not read file.\n");
                return -1;
        }
        int charge_full = strtol(buffer,NULL,10);

        double percentage = ((double)charge_now/(double)charge_full);
        percentage = percentage*100;
        fclose(charge_now_fd);
        fclose(charge_full_fd);
        return percentage;
}
