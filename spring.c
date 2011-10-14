#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static GtkWidget *window;
static GtkWidget *textbox;

static gboolean enter_callback(GtkWidget *widget, GtkWidget *entry, gpointer data);
static void errout(int status, char *str);
static gboolean handle_keypress(GtkWidget *widget, GtkWidget *ev, gpointer data);
static gboolean killevent(GtkWidget *widget, GtkWidget *ev, gpointer data);
static char** split_string(char *str);

static gboolean
enter_callback(GtkWidget *widget, GtkWidget *entry, gpointer data) {
    char *from_field;
    char **splitstring;

    from_field = (char*)gtk_entry_get_text(GTK_ENTRY(entry));
    if (!from_field)
        return FALSE;

    splitstring = split_string(from_field);

    if (fork() == 0) {
        execvp(splitstring[0], splitstring);
        errout(1, "Execvp failed.");
    }

    free(from_field);
    exit(0);

    return FALSE;
}

static void
errout(int status, char *str) {
    if (str)
        puts(str);

    exit(status);
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
    gtk_widget_set_size_request(window, 800, 0);
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
