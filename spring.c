#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXARGS 32

static GtkWidget *window;
static GtkWidget *textbox;
static char **binlist;

static gboolean enter_callback(GtkWidget *widget, GtkWidget *entry, gpointer data);
static void errout(int status, char *str);
static void fill_bin_list(void);
static gboolean handle_keypress(GtkWidget *widget, GtkWidget *ev, gpointer data);
static gboolean killevent(GtkWidget *widget, GtkWidget *ev, gpointer data);
static char** split_string(char *str, char *delim, unsigned int maxtok);

static gboolean
enter_callback(GtkWidget *widget, GtkWidget *entry, gpointer data) {
    char *from_field;
    char **splitstring;

    from_field = (char*)gtk_entry_get_text(GTK_ENTRY(entry));
    if (!from_field)
        return FALSE;

    splitstring = split_string(from_field, " ", MAXARGS);

    if (fork() == 0) {
        execvp(splitstring[0], splitstring);
        errout(1, "Execvp failed.");
    }

    free(from_field);
    free(splitstring);
    exit(0);

    return FALSE;
}

static void
errout(int status, char *str) {
    if (str)
        puts(str);

    exit(status);
}

static void
fill_bin_list(void) {
    printf("Found $PATH to be: %s\n", getenv("PATH"));
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
split_string(char *str, char *delim, unsigned int maxtok) {
    char *tmp[maxtok];
    char **ret;
    int i;

    tmp[0] = strtok(str, delim);
    for (i = 1; (tmp[i] = strtok(NULL, delim)) && i < MAXARGS; ++i);

    if (i == MAXARGS)
        errout(1, "Too many args given");

    ret = malloc(sizeof(char*) * ++i);
    if (!ret)
        errout(1, "Error allocating space for tokens!");

    for (i = 0; tmp[i]; ++i)
        ret[i] = tmp[i];
    ret[i] = NULL;

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

    fill_bin_list();

    gtk_main();

    return 0;
}
