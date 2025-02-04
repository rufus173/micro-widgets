#include <gtk/gtk.h>
void activate(GtkApplication *app, gpointer user_data);
int main(int argc, char **argv){
	GtkApplication *app = gtk_application_new("com.github.rufus172.microTimer",0);
	g_signal_connect(app,"activate",G_CALLBACK(activate), NULL);
	int result = g_application_run(G_APPLICATION(app),argc,argv);
	g_object_unref(app);
	return result;
}
void activate(GtkApplication *app, gpointer user_data){
	GtkWidget *main_window;
	main_window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(main_window),"timer");
	gtk_window_present(GTK_WINDOW(main_window));
}
