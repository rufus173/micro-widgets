#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <gtk/gtk.h>

#define DATE_F_STRING "%d/%m/%y"
#define TIME_F_STRING "%H:%M"

time_t now;
struct tm *time_struct;

char *get_string_date();
char *get_string_time();
void update_time_struct();
int create_window(GtkApplication *app,gpointer user_data){

	//create window
	GtkWidget *window;
	window = gtk_application_window_new(app);
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
