#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <gtk/gtk.h>

#define DATE_F_STRING "%d/%m/%y"
#define TIME_F_STRING "%H:%M"

time_t now;
struct tm *time_struct;
struct label_container {
	GtkWidget *date_label;
	GtkWidget *time_label;
};

char *get_string_date();
char *get_string_time();
void update_time_struct();
int update_loop(gpointer user_data);
int create_window(GtkApplication *app,gpointer user_data){

	//create window
	GtkWidget *window;
	window = gtk_application_window_new(app);
	
	//create a box to put widgets in
	GtkWidget *box;
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_window_set_child(GTK_WINDOW(window), box);
	//centre the box
	gtk_widget_set_halign(box,GTK_ALIGN_CENTER);
	gtk_widget_set_valign(box,GTK_ALIGN_CENTER);

	//make the labels
	struct label_container label_container;
	label_container.date_label = gtk_label_new("Getting date...");
	label_container.time_label = gtk_label_new("Getting time...");
	
	//fill box
	gtk_box_append(GTK_BOX(box), label_container.date_label);
	gtk_box_append(GTK_BOX(box), label_container.time_label);
	
	//set up timer
	g_timeout_add(1000,update_loop,&label_container);

	//present the window
	gtk_window_present(GTK_WINDOW(window));
}
int main(int argc, char **argv){
	printf("date: %s, time: %s\n",get_string_date(),get_string_time());
	//setup the gtk app
	GtkApplication *app;
	app = gtk_application_new("com.github.rufus173.DigitalClock",G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app,"activate",G_CALLBACK(create_window),NULL);
	g_application_run(G_APPLICATION(app),argc,argv);
	g_object_unref(app);
	return 0;
}
int update_loop(gpointer user_data){
	struct label_container *label_container;
	label_container = user_data;
	gchar *text = g_strdup_printf("%s",get_string_date);
	gtk_label_set_label(GTK_LABEL(label_container->date_label),text);
	g_free(text);
	return G_SOURCE_CONTINUE;
}
void update_time_struct(){
	time(&now);
	time_struct = localtime(&now);
}
char *get_string_date(){
	update_time_struct();
	static char time_buffer[1024];
	int status = strftime(time_buffer,1024,DATE_F_STRING,time_struct);
	return time_buffer;
}
char *get_string_time(){
	update_time_struct();
	static char time_buffer[1024];
	int status = strftime(time_buffer,1024,TIME_F_STRING,time_struct);
	return time_buffer;
}
