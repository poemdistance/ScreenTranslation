#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include "printWithColor.h"
#include "configControl.h"
#include "settingWindowData.h"
#include "settingList.h"
#include "expanduser.h"

#define ICON_MARGIN_START (5)

GtkWidget *items[LIST_BOX_ITEMS] = { '\0' };

char settingItems[][128] = {
    "Keyboard Shortcuts",
    "Delay",
    "Icon Position",
    "Window Position",
    "Window Preferences",
};

char settingItemsIcon[][128] = {
    "keyboard-shortcuts",
    "delay",
    "location",
    "window-position",
    "window-preference",
    /* "emblem-system", */
};

int getListBoxItemsNum() {
    int i=0;
    while ( items[i++] );
    return i-1;
}

int insertListBoxItem ( GtkWidget *child, int position ) {

    int i = getListBoxItemsNum();
    if ( i >= LIST_BOX_ITEMS )
        return 0;

    /* position==-1, append child to items[]*/
    if ( position == -1 ) {
        items[i] = child;
        return 1;
    }

    GtkWidget *tmpItems[LIST_BOX_ITEMS] = { '\0' };
    for ( int j=0, k=0; j<i+1; j++ )
        tmpItems[j] = (j==position) ? child : items[k++];

    for ( int j=0; j<i+1; j++)
        items[j] = tmpItems[j];
    return 1;
}

int dropListBoxItem ( int position ) {

    GtkWidget *tmpItems[LIST_BOX_ITEMS] = { '\0' };
    int k  = 0;
    int j = 0;

    while ( items[k] ) {
        if ( k != position )
            tmpItems[j++] = items[k];
        k++;
    }

    j = 0;
    memset ( items, '\0', sizeof(items) );
    while ( tmpItems[j] ) {
        items[j] = tmpItems[j];
        j++;
    }

    return 1;
}

int getListBoxItemIndex( GtkWidget *item , SettingWindowData *settingWindowData) {

    for ( int i = 0; i<sizeof(items)/sizeof(GtkWidget*); i++ ) {
        if ( item == items[i] ) {
            if ( settingWindowData->searchEntryShow )
                return i-1;
            return i;
        }
    }

    return -1;
}

void initSettingItems ( GtkWidget *parent ) {

    GtkWidget *label;
    GtkWidget *icon;
    GtkWidget *grid;
    GtkWidget *box;
    gchar iconPath[512];

    for ( int i = 0; i<sizeof(settingItems)/sizeof(settingItems[0]); i++ ) {

        grid = gtk_grid_new();

        if ( ! insertListBoxItem ( grid, -1 ) ) {
            printf("appendListBoxItems() failed");
            return;
        }

        strcpy ( iconPath, expanduser("/home/$USER/.stran/") );
        strcat ( iconPath, settingItemsIcon[i] );
        strcat ( iconPath, ".png" );

        icon = gtk_image_new_from_file ( iconPath );
        label = gtk_label_new ( settingItems[i] );
        box = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL ,0 );

        gtk_grid_attach ( GTK_GRID(grid), box, 0, 0, 1, 1 );
        gtk_box_pack_start ( GTK_BOX(box), icon, 1, 1, 1 );

        /* 尺寸不够容纳icon将导致对齐出问题*/
        gtk_widget_set_size_request ( box, 50, -1 ); 

        /* separator可以帮助显示哪里出现了对其问题*/
        /* gtk_grid_attach ( GTK_GRID(grid), gtk_separator_new(GTK_ORIENTATION_VERTICAL), 1, 0, 1, 1 ); */

        box = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL ,0 );
        gtk_grid_attach ( GTK_GRID(grid), box, 2, 0, 1, 1 );
        gtk_box_pack_start ( GTK_BOX(box), label, 1, 1, 1 );

        gtk_grid_set_column_spacing ( GTK_GRID(grid), 30 );

        gtk_widget_set_vexpand ( icon, TRUE );
        gtk_widget_set_hexpand ( icon, FALSE );
        gtk_widget_set_margin_start ( icon, ICON_MARGIN_START );
        gtk_widget_set_vexpand ( label, TRUE );

        gtk_widget_set_valign ( icon, GTK_ALIGN_CENTER );
        gtk_widget_set_valign ( label, GTK_ALIGN_CENTER );
        gtk_widget_set_halign ( icon, GTK_ALIGN_CENTER );
        gtk_widget_set_halign ( label, GTK_ALIGN_START );

        /* gtk_widget_set_valign ( icon, GTK_ALIGN_START ); */
        /* gtk_widget_set_valign ( label, GTK_ALIGN_CENTER ); */

        gtk_widget_set_size_request ( grid, -1, LISTBOX_ROW_HEIGHT );
        gtk_list_box_insert ( GTK_LIST_BOX(parent), grid, -1 );
    }
}

