#include <stdio.h>
#include <gtk/gtk.h>
#include <zip.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>

char rom[0xFFFF];
struct passwd *pw;
GtkWidget *window;
GtkWidget *fixed;
GtkBuilder *builder;
GtkDialog *cfg;
GtkTextView *dotcfg;
GtkTextBuffer *cfgbuff;
void delete_ev(GtkWidget *e)
{
    gtk_widget_hide_on_delete((GtkWidget *)cfg);
}
void quit(GtkWidget *e)
{
    g_print("Cerrando...\n");
    killpg(0, SIGTERM);
    gtk_main_quit();
}
int main(int argc, char **argv)
{
    pw = getpwuid(getuid());
    strcpy(rom, pw->pw_dir);
    strcat(rom, "/rom.n64");
    gtk_init(&argc, &argv);
    builder = gtk_builder_new_from_file("gui.glade");
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    gtk_builder_connect_signals(builder, NULL);
    fixed = GTK_WIDGET(gtk_builder_get_object(builder, "fixed"));
    cfg = GTK_DIALOG(gtk_builder_get_object(builder, "cfg"));
    dotcfg = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "dotcfg"));
    gtk_window_set_title((GtkWindow *)window, "Mupen64plus-GTK");
    gtk_window_set_title((GtkWindow *)cfg, "Editor de configuraciones");
    GtkWidget *save = gtk_dialog_add_button(cfg, "Guardar", GTK_RESPONSE_ACCEPT);
    GtkWidget *cancel = gtk_dialog_add_button(cfg, "Cancelar", GTK_RESPONSE_CANCEL);
    gtk_window_set_transient_for((GtkWindow *)cfg, (GtkWindow *)window);
    cfgbuff = gtk_text_view_get_buffer(dotcfg);
    GtkImage *bg = GTK_IMAGE(gtk_builder_get_object(builder, "bg"));
    gtk_image_set_from_file(bg, "icon.png");
    g_signal_connect(window, "destroy", G_CALLBACK(quit), NULL);
    g_signal_connect(cfg, "delete-event", G_CALLBACK(delete_ev), NULL);
    gtk_widget_show(window);
    gtk_main();
    return 0;
}
void on_editcfg_activate(GtkWidget *e)
{
    GtkTextIter end;
    GtkTextIter start;
    FILE *fpr;
    char str[1000];
    char *filename = malloc(sizeof(char) * (strlen("/.config/mupen64plus/mupen64plus.cfg") + strlen(pw->pw_dir) + 2));
    strcpy(filename, pw->pw_dir);
    strcat(filename, "/.config/mupen64plus/mupen64plus.cfg");
    fpr = fopen(filename, "r");
    if (fpr == NULL)
    {
        printf("No existe el archivo %s", filename);
        free(filename);
        return;
    }
    gtk_text_buffer_get_end_iter(cfgbuff, &end);
    gtk_text_buffer_get_start_iter(cfgbuff, &start);
    gtk_text_buffer_delete(cfgbuff, &start, &end);
    while (fgets(str, 1000, fpr) != NULL)
    {
        gtk_text_buffer_get_end_iter(cfgbuff, &end);
        gtk_text_buffer_insert(cfgbuff, &end, str, -1);
    }
    gint res = gtk_dialog_run(cfg);
    if (res == GTK_RESPONSE_ACCEPT)
    {
        FILE *fpw = fopen(filename, "w+");
        gtk_text_buffer_get_end_iter(cfgbuff, &end);
        gtk_text_buffer_get_start_iter(cfgbuff, &start);
        gchar *text = gtk_text_buffer_get_text(cfgbuff, &start, &end, FALSE);
        fprintf(fpw, "%s", text);
        fclose(fpw);
    }
    else if (res == GTK_RESPONSE_CANCEL)
    {
        g_print("ignore\n");
    }
    fclose(fpr);
    free(filename);
    gtk_widget_hide((GtkWidget *)cfg);
}
void open_rom(GtkWidget *e)
{
    GtkWidget *dialog;
    GtkFileFilter *filter;

    gint res;
    dialog = gtk_file_chooser_dialog_new("Abrir ROM", (GtkWindow *)window, GTK_FILE_CHOOSER_ACTION_OPEN, ("Cancelar"), GTK_RESPONSE_CANCEL,
                                         ("Abrir"),
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.[zZ][iI][pP]");
    gtk_file_filter_add_pattern(filter, "*.[nN][6][4]");
    gtk_file_filter_add_pattern(filter, "*.[zZ][6][4]");
    gtk_file_filter_set_name(filter, "N64 ROMs");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        //Open the ZIP archive
        int err = 0;
        struct zip *z = zip_open(filename, 0, &err);
        if (err == ZIP_ER_NOZIP)
        {
            if (fork() == 0)
            {
                execl("/usr/bin/mupen64plus", "64", filename, 0);
            }
        }
        else
        {
            //Search for the file of given name
            const char *name = zip_get_name(z, 0, 0);
            struct zip_stat st;
            zip_stat_init(&st);
            zip_stat(z, name, 0, &st);
            //Alloc memory for its uncompressed contents
            char *contents = malloc(sizeof(char) * st.size);
            //Read the compressed file
            struct zip_file *f = zip_fopen(z, name, 0);
            zip_fread(f, contents, st.size);
            zip_fclose(f);
            FILE *fp = fopen(rom, "w");
            fwrite(contents, sizeof(char), st.size, fp);
            fclose(fp);
            //And close the archive
            zip_close(z);
            free(contents);
            if (fork() == 0)
            {
                execl("/usr/bin/mupen64plus", "64", rom, 0);
            }
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}
