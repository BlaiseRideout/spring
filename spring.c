#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static GtkWidget *window;
static GtkWidget *textbox;

static gboolean enter_callback(GtkWidget *widget, GtkWidget *entry, gpointer data);
static void execute(char *str);
static gboolean handle_keypress(GtkWidget *widget, GtkWidget *ev, gpointer data);
static gboolean killevent(GtkWidget *widget, GtkWidget *ev, gpointer data);
static char** split_string(char *str);

static gboolean
enter_callback(GtkWidget *widget, GtkWidget *entry, gpointer data) {
    char *entry_text;

    entry_text = (char*)gtk_entry_get_text(GTK_ENTRY(entry));
    execute(entry_text);

    return FALSE;
}

static void
execute(char *str) {
    char **splitstring;

    if (!str)
        return;

    splitstring = split_string(str);

    if (fork() == 0) {
        execvp(splitstring[0], splitstring);
        exit(0);
    }

    exit(0);
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

static char**
split_string(char *str) {
    char **ret;
    int i;

    ret = malloc(sizeof(char*) * 32);
    if (!ret[0])
        return ret;

    ret[0] = strtok(str, " ");
    for (i = 1; (ret[i] = strtok(NULL, " ")); ++i);

    return ret;
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
    g_signal_connect(textbox, "activate", G_CALLBACK(enter_callback), textbox);

    gtk_widget_show(textbox);
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
