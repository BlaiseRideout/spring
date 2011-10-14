#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

static GtkWidget *window;
static GtkWidget *textbox;
static char **binlist;

static gboolean enter_callback(GtkWidget *widget, GtkWidget *entry, gpointer data);
static void errout(int status, char *str);
static void fill_bin_list(void);
static gboolean handle_keypress(GtkWidget *widget, GdkEventKey *ev, gpointer data);
static gboolean killevent(GtkWidget *widget, GtkWidget *ev, gpointer data);
static char** split_string(char *str, char *delim);
static int tok_count(char *str, char *delim);

static gboolean
enter_callback(GtkWidget *widget, GtkWidget *entry, gpointer data) {
    char *from_field;
    char **splitstring;
    int i;

    from_field = (char*)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(entry));
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

    /* free(from_field); */
    g_free(splitstring);
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
    char **pathparts;
    DIR *d;
    struct dirent *dir;
    unsigned int bincount = 0;
    int i, e;
    
    /* If we don't do this strtok modifies the environmental variable "PATH" in place and */
    /* messes up the program's ability to execute commands */
    ORIGPATH = getenv("PATH");
    PATH = malloc(strlen(ORIGPATH) + 1);
    if (!PATH)
        errout(1, "Failed to allocate space for in-place PATH chopping.");
    strcpy(PATH, ORIGPATH);

    printf("Found $PATH to be: %s\n", PATH);
    pathparts = split_string(PATH, ":");

    /* Once through is just for counting */
    for (i = 0; pathparts[i]; ++i) {
        d = opendir(pathparts[i]);
        while ((dir = readdir(d))) {
            /* If it is "." or ".." */
            if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
                continue;
            bincount++;
        }
        closedir(d);
    }

    printf("Identified %d binary files.\n", bincount);
    binlist = malloc(sizeof(char*) * ++bincount);
    if (!binlist)
        errout(1, "Error allocating space for the binlist.");
    binlist[--bincount] = NULL;

    /* this time we allocate */
    for (i = 0, e = 0; pathparts[i]; ++i) {
        printf("Path part %d: %s\n", i, pathparts[i]);

        d = opendir(pathparts[i]);
        while ((dir = readdir(d))) {
            /* If it is "." or ".." */
            if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
                continue;

            /* Logic for dealing with found folders */
            binlist[e] = malloc(strlen(dir->d_name) + 1);
            strcpy(binlist[e], dir->d_name);

            ++e;
        }
        closedir(d);
    }

    puts("Identified these binary files:");
    for(i = 0; binlist[i]; ++i)
        printf("- - - >    %s\n", binlist[i]);

    free(PATH);
}

static gboolean
handle_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data) {

    puts("Ding!");

    if (event->keyval == GDK_Escape)
        puts("Enter pressed!");

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
        /* If we found a delim character in this space */
        if (e != delims) {
            ++ret;
            ++i;
            /* Handle any fast-forwarding we need to do */
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
    gtk_widget_set_size_request(window, 800, 10);
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);
    gtk_window_set_policy(GTK_WINDOW(window), FALSE, FALSE, FALSE);

    /* Text entry widget */
    /* textbox = gtk_entry_new(); */
    textbox = gtk_combo_box_text_new_with_entry();
    gtk_container_add(GTK_CONTAINER (window), textbox);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(textbox), "xterm");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(textbox), "chromium");

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
