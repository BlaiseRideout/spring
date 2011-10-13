#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

static GtkWidget *window;
static GtkWidget *textEntry;

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(window, 800, 30);
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
