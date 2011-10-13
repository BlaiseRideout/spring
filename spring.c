#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

static GtkWidget *window;
static GtkWidget *textbox;

static gboolean handle_keypress(GtkWidget *widget, GtkWidget *ev, gpointer data);
static gboolean killevent(GtkWidget *widget, GtkWidget *ev, gpointer data);

static gboolean
enter_callback(GtkWidget *widget, GtkWidget *entry) {
    gchar *entry_text;
    entry_text = gtk_entry_get_text(GTK_ENTRY(entry));
    printf("Entry contents: %s\n", entry_text);
    return FALSE;
}

static gboolean
handle_keypress(GtkWidget *widget, GtkWidget *ev, gpointer data) {

    return FALSE;
}

static gboolean
killevent(GtkWidget *widget, GtkWidget *ev, gpointer data) {
    gtk_main_quit ();
    return FALSE;
}

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    /* Window widget */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(window, 800, 30);
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);
    gtk_window_set_policy(GTK_WINDOW(window), FALSE, FALSE, FALSE);

    /* Text entry widget */
    textbox = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER (window), textbox);

    /* Signal handling */
    g_signal_connect(window, "delete-event", G_CALLBACK(killevent), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(handle_keypress), NULL);
    gtk_signal_connect(GTK_OBJECT(textbox), "activate", GTK_SIGNAL_FUNC(enter_callback), textbox);

    gtk_widget_show(textbox);
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
