/* Copyright (C) 2011 John Anthony */
/*  */
/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */
/*  */
/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */
/*  */
/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*  */

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
static GtkEntry *textbox;
static GtkEntryCompletion *textcompletion;
static GtkListStore *bintree;
static char **binlist;

static void cleanup(void);
static void errout(int status, char *str);
static void fill_bin_list(void);
static gboolean handle_keypress(GtkWidget *widget, GdkEventKey *ev, gpointer data);
static gboolean killevent(GtkWidget *widget, GtkWidget *ev, gpointer data);
static char** split_string(char *str, char *delim);
static gboolean text_exec(void);
static int tok_count(char *str, char *delim);

static void
cleanup(void) {
    gtk_main_quit();
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

    if (event->keyval == GDK_Escape) {
        cleanup();
        exit(0);
    }
    else if (event->keyval == GDK_Return) {
        if (text_exec())
            return TRUE;
        else
            puts("Exec failed!");
    }
    else if (event->keyval == GDK_Tab) {

        return TRUE;
    }

    return FALSE;
}

static gboolean
killevent(GtkWidget *widget, GtkWidget *ev, gpointer data) {
    cleanup();
    exit(0);
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

static gboolean
text_exec(void) {
    char *from_field;
    char **splitstring;
    int i;

    from_field = (char*)gtk_entry_get_text(textbox);
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

int main(int argc, char **argv) {
    unsigned int i;
    GtkTreeIter iter;

    gtk_init(&argc, &argv);

    fill_bin_list();

    /* Window widget */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(window, 800, 10);
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);
    gtk_window_set_policy(GTK_WINDOW(window), FALSE, FALSE, FALSE);

    bintree = gtk_list_store_new(1, G_TYPE_STRING);

    for (i = 0; binlist[i]; ++i) {
        gtk_list_store_append(bintree, &iter);
        gtk_list_store_set(bintree, &iter, 0, binlist[i], -1);
    }

    textcompletion = gtk_entry_completion_new();
    gtk_entry_completion_set_model(textcompletion, GTK_TREE_MODEL(bintree));
    gtk_entry_completion_set_text_column(textcompletion, 0);

    /* Text entry widget */
    textbox = GTK_ENTRY(gtk_entry_new());
    /* textbox = gtk_combo_box_text_new_with_entry(); */
    gtk_entry_set_completion(textbox, textcompletion);
    /* gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(textbox), "xterm"); */
    /* gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(textbox), "chromium"); */
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(textbox));

    /* Signal handling */
    g_signal_connect(window, "delete-event", G_CALLBACK(killevent), NULL);
    g_signal_connect(window, "key-press-event", G_CALLBACK(handle_keypress), NULL);

    gtk_widget_show(GTK_WIDGET(textbox));
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
