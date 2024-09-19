#include "gtk4-layer-shell.h"
#include <gtk/gtk.h>

static void activate (GtkApplication* app, void *_data) {
    (void)_data;

    GtkWindow *gtk_window = GTK_WINDOW (gtk_application_window_new (app));

    gtk_layer_init_for_window (gtk_window);

    // 레이어 설정
    gtk_layer_set_layer (gtk_window, GTK_LAYER_SHELL_LAYER_TOP);

    // 자동 독점 영역 비활성화
    // gtk_layer_auto_exclusive_zone_enable (gtk_window); // 주석 처리 또는 제거

    // 윈도우 크기를 정사각형으로 설정
    gtk_window_set_default_size(gtk_window, 300, 300); // 예: 300x300

    // 윈도우를 화면 중앙에 배치
    gtk_window_set_gravity(gtk_window, GDK_GRAVITY_CENTER);
    GtkWidget *label = gtk_label_new ("");
    gtk_label_set_text(GTK_LABEL(label),
"Hello, World!");
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