#include "gtk4-layer-shell.h"
#include <gtk/gtk.h>

static void activate (GtkApplication* app, void *_data) {
    (void)_data;

    // Create a normal GTK window however you like
    GtkWindow *gtk_window = GTK_WINDOW (gtk_application_window_new (app));

    // Before the window is first realized, set it up to be a layer surface
    gtk_layer_init_for_window (gtk_window);

    // Order below normal windows
    gtk_layer_set_layer (gtk_window, GTK_LAYER_SHELL_LAYER_TOP);

    // Push other windows out of the way
    gtk_layer_auto_exclusive_zone_enable (gtk_window);
    // Set up a widget
    GtkWidget *label = gtk_label_new ("");
    gtk_label_set_markup (GTK_LABEL (label),
                          "<span font_desc=\"100.0\">"
                              "GTK Layer\nShell example!"
                          "</span>");
    gtk_window_set_child (gtk_window, label);
    gtk_window_present (gtk_window);
}

int main (int argc, char **argv) {
    GtkApplication * app = gtk_application_new ("gtk.popup", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    int status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return status;
}