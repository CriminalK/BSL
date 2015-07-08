//#include <stdio.h>
//#include <gtk/gtk.h>
//
//#include "bsl-settings.h"
//
//static void activate(GtkApplication* app, gpointer user_data)
//{
//  GtkWidget * window;
//  GtkWidget * bsl_settings;
//
//  window = gtk_application_window_new(app);
//  gtk_window_set_title(GTK_WINDOW(window), "Window");
//  gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
//
//  bsl_settings = bsl_settings_new();
//  gtk_container_add(GTK_CONTAINER(window), bsl_settings);
//
//  gtk_widget_show_all (window);
//}
//
//int main(int argc, char **argv)
//{
//  GtkApplication *app;
//  int status;
//
//  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
//  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
//  status = g_application_run (G_APPLICATION (app), argc, argv);
//  g_object_unref (app);
//
//  return status;
//}
