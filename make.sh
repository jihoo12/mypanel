gcc -o popup popup.c $(pkg-config --cflags --libs gtk4) -L. -lgtk4-layer-shell
