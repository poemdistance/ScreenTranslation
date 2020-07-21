#include <gtk/gtk.h>
#include <X11/XKBlib.h>
#include "settingWindowData.h"
#include "panel.h"
#include "expanduser.h"
#include "printWithColor.h"
#include "configControl.h"
#include "useful.h"

static void addMouseActioncutToListBox ( 
        char (*mouseActionItems)[SHORTCUT_CONTENT_LEN],
        char (*key)[SHORTCUT_CONTENT_LEN],
        SettingWindowData *settingWindowData ) {

    GtkListBox *list = (GtkListBox*)settingWindowData->mouseActionSettingWindowData->listBox;
    GtkWidget *grid;
    GtkWidget *box;
    GtkComboBox *combo_box;
    GtkTreeIter iter;
    int valid;
    int i;
    GtkListStore * list_store;
    GtkWidget *label;
    GtkWidget *image;
    GtkWidget *button;

    /* grid->box->label grid->box->label*/
    for ( int i=0; i<MAX_SHORTCUT_NUM; i++ ) {

        if ( *mouseActionItems[i] ) {

            grid = gtk_grid_new();
            gtk_widget_set_size_request ( grid, -1, LISTBOX_ROW_HEIGHT );

            /* 条目名称*/
            box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_widget_set_size_request ( box, 250, -1 );

            label = gtk_label_new ( key[i] );
            gtk_grid_attach ( GTK_GRID(grid), box, 0, 0, 1, 1 );
            gtk_box_pack_start ( GTK_BOX(box), label, 1, 1, 1 );
            gtk_widget_set_vexpand ( label, TRUE );
            gtk_widget_set_hexpand ( label, TRUE );
            gtk_widget_set_margin_start ( label, MARGIN_START );
            gtk_widget_set_halign ( label, GTK_ALIGN_START );

            /* 条目选项值*/
            /* Create the combo box and append your string values to it. */
            combo_box = gtk_combo_box_text_new ();
            const char *distros[] = {"Select distribution", "Fedora", "Mint", "Suse"};

            /* G_N_ELEMENTS is a macro which determines the number of elements in an array.*/
            for (i = 0; i < G_N_ELEMENTS (distros); i++){
                gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo_box), distros[i]);
            }

            /* Choose to set the first row as the active one by default, from the beginning */
            gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), 0);

            gtk_widget_set_size_request ( combo_box, 100, -1 );
            label = gtk_label_new ( mouseActionItems[i] );
            gtk_widget_set_margin_end ( label, MARGIN_END );
            gtk_grid_attach ( GTK_GRID(grid), combo_box, 1, 0, 1, 1 );

            gtk_grid_set_column_spacing ( GTK_GRID(grid), 36 );

            gtk_list_box_insert ( GTK_LIST_BOX(list), grid, -1 );
        }
    }

    /* g_signal_connect ( list, "row-activated", */
    /*         G_CALLBACK(on_listbox_row_selected_cb), settingWindowData ); */

    gtk_widget_show_all ( settingWindowData->contentScrollWindow );
}

void mouseActionSetting( SettingWindowData *settingWindowData ) {

    static MouseActionSettingWindowData maswd;
    memset ( &maswd, '\0', sizeof(MouseActionSettingWindowData) );
    settingWindowData->mouseActionSettingWindowData = &maswd;

    /* Remove all the previous widgets in container*/
    gtk_container_forall (
            (GtkContainer*)settingWindowData->contentScrollWindow,
            remove_widget,
            (settingWindowData->contentScrollWindow));

    /* GTK BOX*/
    GtkWidget *box = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 ); 
    gtk_container_add ( GTK_CONTAINER(settingWindowData->contentScrollWindow), box );
    gtk_widget_set_hexpand ( box, TRUE  );
    gtk_widget_set_size_request ( settingWindowData->contentScrollWindow, 400, 400 );
    gtk_widget_set_margin_start(box, MARGIN_START );
    gtk_widget_set_margin_end(box, MARGIN_END );
    gtk_widget_set_margin_top(box, MARGIN_TOP );

    /* GTK LIST BOX */
    GtkWidget *listBox = gtk_list_box_new();
    settingWindowData->mouseActionSettingWindowData->listBox = listBox;
    gtk_box_pack_start ( GTK_BOX(box), listBox, 0,0,0 );
    gtk_widget_set_halign ( listBox, GTK_ALIGN_CENTER );

    static char mouseActionItems[MAX_SHORTCUT_NUM][SHORTCUT_CONTENT_LEN];
    static char options[MAX_SHORTCUT_NUM][SHORTCUT_CONTENT_LEN];
    memset ( mouseActionItems, '\0', sizeof(mouseActionItems) );

    readFromConfigByKeyword ( mouseActionItems, "MouseAction" );
    readNameByKeyword ( options, "MouseAction" );

    addMouseActioncutToListBox ( mouseActionItems, options, settingWindowData ); 

    gtk_widget_show_all ( settingWindowData->contentScrollWindow );
}
