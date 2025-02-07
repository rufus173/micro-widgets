#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

//======= struct defines =======
struct timer_instance {
	time_t start_time;
	time_t end_time;
	unsigned int timer_id;
	GtkWidget *window;
	GtkWidget *progress_bar;
	GtkWidget *time_remaining_label;
	LIST_ENTRY(timer_instance) entries;
};

//======== globals ========
LIST_HEAD(timer_instances_head, timer_instance);
struct timer_instances_head timer_instances_head;

GtkAdjustment *seconds_adjustment;
GtkAdjustment *minutes_adjustment;
GtkAdjustment *hours_adjustment;

void activate(GtkApplication *app, gpointer user_data);
void start_new_timer_callback();
void cleanup_timer_callback(struct timer_instance *instance);
gboolean update_timer(struct timer_instance *instance);
int main(int argc, char **argv){
	LIST_INIT(&timer_instances_head); //this is completely pointless btw just wanted to try it out
	GtkApplication *app = gtk_application_new("com.github.rufus172.microTimer",0);
	g_signal_connect(app,"activate",G_CALLBACK(activate), NULL);
	int result = g_application_run(G_APPLICATION(app),argc,argv);
	g_object_unref(app);
	return result;
}
void activate(GtkApplication *app, gpointer user_data){
	//========== setup of window =========
	GtkWidget *main_window;
	main_window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(main_window),"timer");
	GtkWidget *main_window_grid = gtk_grid_new();
	gtk_window_set_child(GTK_WINDOW(main_window),main_window_grid);

	//========== start button ==========
	GtkWidget *start_timer_button = gtk_button_new_with_label("start");
	gtk_grid_attach(GTK_GRID(main_window_grid),start_timer_button,0,0,3,1);
	g_signal_connect_swapped(start_timer_button,"clicked",G_CALLBACK(start_new_timer_callback),app); //connect swapped because we dont care about the button object

	//========== time selection labels ==========
	GtkWidget* seconds_label = gtk_label_new("seconds");
	GtkWidget* minutes_label = gtk_label_new("minutes");
	GtkWidget* hours_label = gtk_label_new("hours");
	gtk_grid_attach(GTK_GRID(main_window_grid),seconds_label,2,2,1,1);
	gtk_grid_attach(GTK_GRID(main_window_grid),minutes_label,1,2,1,1);
	gtk_grid_attach(GTK_GRID(main_window_grid),hours_label,0,2,1,1);

	//========== time selection boxes ==========
	seconds_adjustment = gtk_adjustment_new(0,0.0,60,1,0,0);
	minutes_adjustment = gtk_adjustment_new(0,0.0,60,1,0,0);
	hours_adjustment = gtk_adjustment_new(0,0.0,60,1,0,0);
	GtkWidget *seconds_box = gtk_spin_button_new(seconds_adjustment,2.0,0);
	GtkWidget *minutes_box = gtk_spin_button_new(minutes_adjustment,2.0,0);
	GtkWidget *hours_box = gtk_spin_button_new(hours_adjustment,2.0,0);
	gtk_grid_attach(GTK_GRID(main_window_grid),seconds_box,2,3,1,1);
	gtk_grid_attach(GTK_GRID(main_window_grid),minutes_box,1,3,1,1);
	gtk_grid_attach(GTK_GRID(main_window_grid),hours_box,0,3,1,1);

	//=========== presenting window ============
	gtk_window_present(GTK_WINDOW(main_window));
}
void cleanup_timer_callback(struct timer_instance *instance){
	printf("timer done\n");

	//stop the timer
	g_source_remove(instance->timer_id);

	gtk_window_destroy(GTK_WINDOW(instance->window)); //kill window

	//clean up
	LIST_REMOVE(instance,entries);
	free(instance);
}
void start_new_timer_callback(GtkApplication *app){
	short hours = gtk_adjustment_get_value(hours_adjustment);
	short minutes = gtk_adjustment_get_value(minutes_adjustment);
	short seconds = gtk_adjustment_get_value(seconds_adjustment);
	printf("new timer for %dh %dm %ds\n",hours,minutes,seconds);
	time_t total_time_s = seconds + (minutes*60) + (hours*3600);

	//prep struct
	struct timer_instance *instance;
	instance = malloc(sizeof(struct timer_instance));
	//insert into doubly linked list so it can be retreived later
	LIST_INSERT_HEAD(&timer_instances_head, instance,entries);
	//set the start and end time
	instance->start_time = time(NULL);
	instance->end_time = time(NULL) + total_time_s;

	//======== window creation ======
	instance->window = gtk_window_new();
	//gtk_window_set_default_size(GTK_WINDOW(instance->window),100,100);
	//connect to cleanup function
	g_signal_connect_swapped(instance->window,"close-request",G_CALLBACK(cleanup_timer_callback),instance);

	//======== widgets ========
	instance->progress_bar = gtk_progress_bar_new();
	instance->time_remaining_label = gtk_label_new("time remaining: ");

	//im not sure i should just be abandoning the pointer but whatever
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	gtk_box_append(GTK_BOX(box),instance->time_remaining_label);
	gtk_box_append(GTK_BOX(box),instance->progress_bar);

	//======== update timer =======
	//                                  1s
	instance->timer_id = g_timeout_add(1000,G_SOURCE_FUNC(update_timer),instance);

	//======== present =======
	gtk_window_set_child(GTK_WINDOW(instance->window),box);
	gtk_window_present(GTK_WINDOW(instance->window));
}
gboolean update_timer(struct timer_instance *instance){
	if (time(NULL) > instance->end_time){
		//stop timer update loop
		return G_SOURCE_REMOVE;
	}
	const time_t time_difference = instance->end_time - time(NULL);
	const float percent_complete = 1-((float)time_difference / (float)(instance->end_time-instance->start_time));
	
	//======== update the progress bar and label =======
	char label_text_buffer[1024];
	strftime(label_text_buffer,sizeof(label_text_buffer),"time remaining: %Hh, %Mm, %Ss",gmtime(&time_difference));
	gtk_label_set_text(GTK_LABEL(instance->time_remaining_label),label_text_buffer);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(instance->progress_bar),percent_complete);

	//keep going
	return G_SOURCE_CONTINUE;
}
