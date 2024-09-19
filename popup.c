#include "gtk4-layer-shell.h"
#include <gtk/gtk.h>

static void activate (GtkApplication* app, void *_data) {
    (void)_data;

    GtkWindow *gtk_window = GTK_WINDOW (gtk_application_window_new (app));

    gtk_layer_init_for_window (gtk_window);

    gtk_window_set_default_size(gtk_window, 300, 300);
    gtk_window_set_gravity(gtk_window, GDK_GRAVITY_CENTER);

    GtkWidget *label = gtk_label_new ("");
    gtk_label_set_text(GTK_LABEL(label), "Hello, World!");

    // 외부 CSS 파일 로드 및 적용
    GtkCssProvider *css_provider = gtk_css_provider_new();
    GError *error = NULL;
    gtk_css_provider_load_from_path(css_provider, "style.css", &error);
    
    if (error) {
        g_warning("Error loading CSS file: %s", error->message);
        g_clear_error(&error);
    } else {
        GtkStyleContext *context = gtk_widget_get_style_context(label);
        gtk_style_context_add_provider(context,
                                       GTK_STYLE_PROVIDER(css_provider),
                                       GTK_STYLE_PROVIDER_PRIORITY_USER);
    }

    g_object_unref(css_provider);

    gtk_window_set_child(gtk_window, label);
    gtk_window_present(gtk_window);
}

int main (int argc, char **argv) {
    GtkApplication * app = gtk_application_new ("gtk.popup", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    int status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
    return status;
}