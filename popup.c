#include "gtk4-layer-shell.h"
#include <gtk/gtk.h>

static void activate (GtkApplication* app, void *_data) {
    (void)_data;

    GtkWindow *gtk_window = GTK_WINDOW (gtk_application_window_new (app));

    gtk_layer_init_for_window (gtk_window);

    gtk_window_set_default_size(gtk_window, 300, 300);

    GtkWidget *label = gtk_label_new ("");
    gtk_label_set_text(GTK_LABEL(label), "Hello, World!");

    gtk_window_set_child(gtk_window, label);
    gtk_window_present(gtk_window);
}


int main (int argc, char **argv) {
    GtkApplication * app = gtk_application_new ("gtk.popup", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    int status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return status;
}
