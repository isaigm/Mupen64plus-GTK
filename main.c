#include <stdio.h>
#include <gtk/gtk.h>
#include <zip.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
static char rom[65536];
static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    g_print("Cerrando...\n");
    killpg(0, SIGTERM);
    gtk_main_quit ();
}
static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    /* If you return FALSE in the "delete-event" signal handler,
     * GTK will emit the "destroy" signal. Returning TRUE means
     * you don't want the window to be destroyed.
     * This is useful for popping up 'are you sure you want to quit?'
     * type dialogs. */

    g_print ("delete event occurred\n");

    /* Change TRUE to FALSE and the main window will be destroyed with
     * a "delete-event". */

    return FALSE;
}
static void open_rom(GtkWidget *widget, gpointer *data){
    GtkWidget *dialog;
    gint res;

    dialog = gtk_file_chooser_dialog_new ("Abrir ROM", NULL, GTK_FILE_CHOOSER_ACTION_OPEN,("Cancelar"), GTK_RESPONSE_CANCEL,
                                          ("Abrir"),
                                          GTK_RESPONSE_ACCEPT,
                                          NULL);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
      {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename (chooser);
        //Open the ZIP archive
        int err = 0;
        struct zip *z = zip_open(filename, 0, &err);
        if(err == ZIP_ER_NOZIP){
            g_print("Sin soporte\n");
        }else{
            //Search for the file of given name
            const char *name = zip_get_name(z, 0, 0);
            struct zip_stat st;
            zip_stat_init(&st);
            zip_stat(z, name, 0, &st);
            //Alloc memory for its uncompressed contents
            char *contents = malloc(sizeof (char) * st.size);
            //Read the compressed file
            printf("%s\n", name);
            struct zip_file *f = zip_fopen(z, name, 0);
            zip_fread(f, contents, st.size);
            zip_fclose(f);
            FILE *fp = fopen(rom, "w");
            fwrite(contents, sizeof (char), st.size, fp);
            fclose(fp);
            //And close the archive
            zip_close(z);
            free(contents);
            if (fork() == 0) {
                execl("/usr/bin/mupen64plus", "64", rom, 0);
            }
       }
        g_free (filename);
      }
    gtk_widget_destroy (dialog);
}
int main(int argc, char **argv)
{
    struct passwd *pw = getpwuid(getuid());
    strcpy(rom, pw->pw_dir);
    strcat(rom, "/rom.n64");
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *menubar;
    GtkWidget *fileMenu;
    GtkWidget *fileMi;
    GtkWidget *romMi;
    GtkWidget *quitMi;
    gtk_init(&argc, &argv);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW (window), "Mupen64Plus-GTK");
    gtk_window_set_default_size(GTK_WINDOW (window), 800, 600);
    gtk_window_set_position(GTK_WINDOW (window), GTK_WIN_POS_CENTER);
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER (window), vbox);
    menubar = gtk_menu_bar_new();
    fileMenu = gtk_menu_new();
    fileMi = gtk_menu_item_new_with_label("Archivo");
    romMi = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
    quitMi = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);;
    gtk_menu_item_set_submenu(GTK_MENU_ITEM (fileMi), fileMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL (fileMenu), romMi);
    gtk_menu_shell_append(GTK_MENU_SHELL (fileMenu), quitMi);
    gtk_menu_shell_append(GTK_MENU_SHELL (menubar), fileMi);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK (destroy), NULL);
    g_signal_connect(G_OBJECT(quitMi), "activate", G_CALLBACK(destroy), NULL);
    g_signal_connect(G_OBJECT(romMi), "activate", G_CALLBACK(open_rom), NULL);
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
