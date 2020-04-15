#include <gtk/gtk.h>
#include "settingWindowData.h"
#include "panel.h"
#include "expanduser.h"
#include "printWithColor.h"
#include "configControl.h"
#include "useful.h"

static void on_switch_state_change ( 
        GtkSwitch *widget,
        gboolean state,
        char *prefName ) {

    char buf[64];
    strcpy ( buf, prefName );
    strcat ( buf, "-Pref" );

    writeToConfig ( buf, bool2str(state) );

    gtk_switch_set_state ( widget, state );
}

static void addPrefToListBox ( 
        char (*prefValue)[PREF_CONTENT_LEN],
        char (*prefName)[PREF_CONTENT_LEN],
        SettingWindowData *swd ) {

    WinPrefSettingWindowData *wpswd = swd->winPrefSettingWindowData;
    GtkListBox *list = (GtkListBox*)wpswd->listBox;
    GtkWidget *grid;
    GtkWidget *box;
    GtkWidget *label;
    GtkWidget *switchButton;

    for ( int i=0; i<MAX_PREF_NUM; i++ ) {

        if ( *prefValue[i] ) {

            grid = gtk_grid_new();
            gtk_widget_set_size_request ( grid, -1, LISTBOX_ROW_HEIGHT );

            /* 快捷键名称*/
            box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_widget_set_size_request ( box, 225, -1 );

            label = gtk_label_new ( prefName[i] );
            gtk_grid_attach ( GTK_GRID(grid), box, 0, 0, 1, 1 );
            gtk_box_pack_start ( GTK_BOX(box), label, 1, 1, 1 );
            gtk_widget_set_vexpand ( label, TRUE );
            gtk_widget_set_hexpand ( label, TRUE );
            gtk_widget_set_margin_start ( label, MARGIN_START );
            gtk_widget_set_halign ( label, GTK_ALIGN_START );

            /* gtk_grid_attach ( GTK_GRID(grid), gtk_separator_new(GTK_ORIENTATION_VERTICAL), 1, 0, 1, 1 ); */

            /* 快捷键键值*/
            box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
            gtk_widget_set_size_request ( box, 145, -1 );
            switchButton = gtk_switch_new();
            gtk_switch_set_state ( GTK_SWITCH(switchButton), str2bool(prefValue[i]) );
            gtk_widget_set_margin_end ( label, MARGIN_END );
            gtk_grid_attach ( GTK_GRID(grid), box, 1, 0, 1, 1 );
            gtk_box_pack_start ( GTK_BOX(box), switchButton, 1, 1, 1 );

            g_signal_connect ( switchButton, "state-set", 
                    G_CALLBACK(on_switch_state_change), prefName[i]);

            gtk_widget_set_vexpand ( switchButton, FALSE );
            gtk_widget_set_hexpand ( switchButton, FALSE );
            gtk_widget_set_halign ( switchButton, GTK_ALIGN_CENTER );
            gtk_widget_set_valign ( switchButton, GTK_ALIGN_CENTER );

            gtk_grid_set_column_spacing ( GTK_GRID(grid), 100 );

            gtk_list_box_insert ( GTK_LIST_BOX(list), grid, -1 );
        }
    }

    gtk_widget_show_all ( swd->contentScrollWindow );
}


void windowPrefSetting ( SettingWindowData *swd ) {

    static WinPrefSettingWindowData wpswd;
    swd->winPrefSettingWindowData = &wpswd;
    memset ( &wpswd, '\0', sizeof(wpswd) );

    /* Remove all the previous widgets in container*/
    gtk_container_forall (
            (GtkContainer*)swd->contentScrollWindow,
            remove_widget,
            (swd->contentScrollWindow));

    /* GTK BOX*/
    GtkWidget *box = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 ); 
    wpswd.box = box;
    gtk_container_add ( GTK_CONTAINER(swd->contentScrollWindow), box );
    gtk_widget_set_hexpand ( box, TRUE  );
    gtk_widget_set_vexpand ( box, TRUE  );
    gtk_widget_set_margin_start(box, MARGIN_START );
    gtk_widget_set_margin_end(box, MARGIN_END );
    gtk_widget_set_margin_top(box, MARGIN_TOP );

    /* GTK LIST BOX */
    GtkWidget *listBox = gtk_list_box_new();
    wpswd.listBox = listBox;
    gtk_box_pack_start ( GTK_BOX(box), listBox, 0,0,0 );
    gtk_widget_set_halign ( listBox, GTK_ALIGN_CENTER );

    /* 读取所有窗口配置信息和配置名*/
    static char prefName[MAX_PREF_NUM][PREF_CONTENT_LEN];
    static char prefValue[MAX_PREF_NUM][PREF_CONTENT_LEN];
    memset ( prefName, '\0', sizeof(prefName) );
    memset ( prefValue, '\0', sizeof(prefValue) );
    readFromConfigByKeyword ( prefValue, "Pref" );
    readNameByKeyword ( prefName, "Pref" );

    /* 添加窗口配置信息到listBox*/
    addPrefToListBox ( prefValue, prefName, swd ); 

    inline void disable_selectable_activatable( GtkWidget *widget, gpointer data ) {
        gtk_list_box_row_set_selectable ( GTK_LIST_BOX_ROW(widget), false );
        gtk_list_box_row_set_activatable ( GTK_LIST_BOX_ROW(widget), false );
    }

    /* Set all listBoxRow unSeletable and unActivatable*/
    gtk_container_forall ( 
            GTK_CONTAINER(listBox),
            disable_selectable_activatable,
            NULL);

    gtk_widget_show_all ( swd->contentScrollWindow );
}
