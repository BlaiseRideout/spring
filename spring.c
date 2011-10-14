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
static char** split_string(char *str, char *delim);
static int tok_count(char *str, char *delim);

static gboolean
enter_callback(GtkWidget *widget, GtkWidget *entry, gpointer data) {
    char *from_field;
    char **splitstring;
    int i;

    from_field = (char*)gtk_entry_get_text(GTK_ENTRY(entry));
    if (!from_field)
        return FALSE;

    splitstring = split_string(from_field, " ");

    if (fork() == 0) {
        execvp(splitstring[0], splitstring);
        printf("Execvp failed when attempting to launch ");
        for (i = 0; splitstring[i]; ++i)
            printf("%s ", splitstring[i]);
        puts("");
        exit(1);
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
    char *ORIGPATH;
    char *PATH;
    int i;
    
    /* If we don't do this strtok modifies the environmental variable "PATH" in place and */
    /* messes up the program's ability to execute commands */
    ORIGPATH = getenv("PATH");
    PATH = malloc(strlen(ORIGPATH) + 1);
    if (!PATH)
        errout(1, "Failed to allocate space for in-place PATH chopping.");
    strcpy(PATH, ORIGPATH);

    printf("Found $PATH to be: %s\n", PATH);
    binlist = split_string(PATH, ":");

    for (i = 0; binlist[i]; ++i)
        printf("Path part %d: %s\n", i, binlist[i]);

    free(PATH);
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
split_string(char *str, char *delim) {
    char **ret;
    int numtok;
    int i;

    numtok = tok_count(str, delim) + 1;
    ret = malloc(sizeof(char*) * numtok);
    if (!ret)
        errout(1, "Error allocating space for tokens!");

    ret[0] = strtok(str, delim);
    for (i = 1; i < numtok; ++i)
        ret[i] = strtok(NULL, delim);

    if (ret[--i] != NULL)
        errout(1, "split_string() return value not NULL-terminated. tok_count() error?");

    return ret;
}

/* Determines the number of tokens that strtok() will return */
static int
tok_count(char *str, char *delim) {
    int i, e;
    int ret;
    int delims;

    ret = 1;
    delims = strlen(delim);

    for(i = 0; i < strlen(str); ++i) {
        /* Set e to any value but == delims if we're still counting towards one token */
        for (e = 0; e < delims && str[i] != delim[e]; ++e);
        /* Handle any fast-forwarding we need to do */
        if (e != delims) {
            ++ret;
            ++i;
            for (e = 0; e < delims; ++e) {
                if (str[i] == delim[e]) {
                    e = 0;
                    ++i;
                }
            }
        }
    }

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
