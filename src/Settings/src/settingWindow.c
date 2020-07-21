#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include "settingWindowData.h"
#include "panel.h"
#include "focus.h"
#include "gtkLabel.h"
#include "settingList.h"
#include "configControl.h"
#include "useful.h"
#include "printWithColor.h"
#include "expanduser.h"
#include "sharedMemory.h"

/* 添加设置窗口新条目的步骤：
 *
 * 1. 到settingWindowData.h 定义新的结构体,并在SettingWindowData结构体中
 *    定义一个新的该结构体的指针
 * 
 * 2. 添加新的.c文件，入口函数在panel.h中声明,并将该入口函数加入本文件中
 *    contentDisplayFunc[] 数组中.
 *
 * 3. 到settingList.c中加入该条目的标题到settingItems[][]数组, 然后找到
 *    settingItemsIcon[][]数组，加入该条目对应的图标.
 *
 * 4. 将页面逻辑添加到新的.c文件中，可以参考其他文件的书写方式.一般步骤是
 *    先将swd->contentScrollWindow清空，然后在其内部添加各种所需部件.
 *
 * */


extern GtkWidget *items[LIST_BOX_ITEMS];

ContentDisplayFunc contentDisplayFunc[] = {
    shortcutSetting,
    iconShowTimeSetting, 
    iconPositionSetting,
    windowPosSetting,
    windowPrefSetting,
    mouseActionSetting,
};

ContentDisplayFunc getFuncBySelectedRow ( int row ) {

    return contentDisplayFunc[row];
}

void removeLockFile () {

    char *fileName = expanduser("/home/$USER/.stran/settingWindow.lock");
    if ( ! access (  fileName, F_OK) )
        remove ( fileName );
}

void gtk_window_destroy ( GtkWidget *window, SettingWindowData *settingWindowData ) {

    if ( ! settingWindowData->shm )
        shared_memory_for_setting ( &settingWindowData->shm );
    settingWindowData->shm[1] = '0';
    gtk_widget_destroy(window);
    memset ( settingWindowData, '\0', sizeof(SettingWindowData) );
    memset ( items, '\0', sizeof(items) ); /* Necessary*/
    removeLockFile();
}

/* static void on_enter_notify_cb ( */ 
/*         GtkWidget *icon, */
/*         GdkEvent *event, */
/*         SettingWindowData *swd ) { */

/*     IconPositionSettingWindowData *ipswd */ 
/*         = swd->iconPositionSettingWindowData; */

/*     ipswd->iconEnter = TRUE; */
/* } */

gboolean on_configure_event_cb ( 
        GtkWindow *window,
        GdkEvent *event,
        SettingWindowData *settingWindowData ) {

    static int previousHeight, previousWidth;

    int height=0, width=0;
    char h[16] = { '\0' };
    char w[16] = { '\0' };

    gtk_window_get_size ( GTK_WINDOW(window), &width, &height );

    if ( width != previousWidth || height != previousHeight ) {
        writeToConfig ( "Setting-Window-Width", int2str(width, w) );
        writeToConfig ( "Setting-Window-Height", int2str(height, h) );
        /* pmag ( "Write window width=%d height=%d", width, height ); */
    }
    previousWidth = width;
    previousHeight = height;

    settingWindowData->height = height;
    settingWindowData->width = width;
    settingWindowData->previousHeight = height;
    settingWindowData->previousWidth = width;
    /* gtk_widget_queue_draw ( (GtkWidget*)window ); */

    /* FALSE to propagate the event further.*/
    return FALSE;
}

void updateContentScrollWindow ( 
        int row,
        SettingWindowData *settingWindowData ) {

    getFuncBySelectedRow ( row ) ( settingWindowData );
}

static void on_row_selected_call_back ( 
        GtkListBox *listbox,
        GtkListBoxRow *listboxrow,
        SettingWindowData *settingWindowData ) {

    int row = getListBoxItemIndex(
            gtk_bin_get_child(GTK_BIN(listboxrow)),
            settingWindowData);

    if ( row != -1 )
        gtk_window_set_title ( 
                (GtkWindow*) settingWindowData->window,
                settingItems[row] );

    if ( row != -1 )
        updateContentScrollWindow ( row, settingWindowData );
}

gboolean unShowSearchEntry( 
        GtkSearchEntry *searchEntry,
        SettingWindowData *settingWindowData ) {

    settingWindowData->searchEntryShow = false;
    gtk_container_remove (
            (GtkContainer*) settingWindowData ->listBox,
            (GtkWidget*)gtk_list_box_get_row_at_index (
                (GtkListBox*)settingWindowData->listBox, 0  ) );

    dropListBoxItem ( 0 );
    return true;
}


void search_change_call_back ( GtkSearchEntry *entry, gpointer *data ) {

    /* const char *buf; */
    /* buf = gtk_entry_get_text ( (GtkEntry*)entry ); */
}

gboolean search_button_click_call_back ( 
        GtkButton *button,
        SettingWindowData *settingWindowData) {

    /* Remove the searchEntry row in listBox if it has been shown */
    if ( settingWindowData->searchEntryShow )
        return unShowSearchEntry( NULL, settingWindowData );

    GtkWidget *listBox = settingWindowData->listBox;
    GtkWidget *searchEntry = gtk_search_entry_new();
    GtkWidget *searchEntryBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(searchEntryBox), searchEntry, 1, 1, 1);
    gtk_widget_set_size_request ( searchEntryBox, -1, LISTBOX_ROW_HEIGHT );
    gtk_widget_set_size_request ( searchEntry, -1, LISTBOX_ROW_HEIGHT );
    gtk_list_box_insert ( GTK_LIST_BOX( listBox ), searchEntryBox, 0 );
    gtk_widget_grab_focus ( searchEntry );

    insertListBoxItem ( searchEntryBox, 0 );
    settingWindowData->searchEntry = searchEntry;
    settingWindowData->searchEntryBox = searchEntryBox;
    settingWindowData->searchEntryShow = true;

    g_signal_connect ( searchEntry, "stop-search",
            G_CALLBACK(unShowSearchEntry), settingWindowData );
    g_signal_connect ( searchEntry, "search-changed",
            G_CALLBACK(search_change_call_back), settingWindowData );
    gtk_entry_set_placeholder_text ( GTK_ENTRY(searchEntry), "Insert" );
    gtk_entry_set_visibility (  GTK_ENTRY(searchEntry), true  );

    /* Must to call following function or it will show with the wrong thing*/
    gtk_widget_show ( searchEntryBox );
    gtk_widget_show ( searchEntry );

    return TRUE;
}


gboolean on_key_press_cb ( 
        GtkWidget *window,
        GdkEventKey *event,
        SettingWindowData *settingWindowData) {

    if ( event->state ==  GDK_CONTROL_MASK ) {
        if ( event->keyval == GDK_KEY_c) {
            gtk_window_destroy ( window, settingWindowData );
        }

        return true;
    }

    gint keyval = event->keyval;

    if ( keyval & GDK_KEY_Control_L &
            GDK_KEY_BackSpace &
            GDK_KEY_Delete &
            GDK_KEY_Tab &
            GDK_KEY_Alt_R &
            GDK_KEY_Alt_L
       )
        return false;

    if ( ! settingWindowData->searchEntryShow )
        search_button_click_call_back ( NULL, settingWindowData);

    return false;
}

int focusProxy(void *data) {

    static int times = 0;
    
    pgreen ( "Focus Proxy" );

    if ( ++times <= 5 ) {
        focusRequest ( data );
        return TRUE;
    }

    pgreen ( "Canceled keep above" );
    SettingWindowData *swd = data;
    gtk_window_set_keep_above ( GTK_WINDOW(swd->window), FALSE );
    return FALSE;
}

void settingWindow(
        GtkWidget  *menuItemSetting,
        SettingWindowData *settingWindowData) {

    SettingWindowData *swd = settingWindowData;

    char *fileName = expanduser("/home/$USER/.stran/settingWindow.lock");

    if ( ! access ( fileName, F_OK ) ) {
        pmag ( "窗口已经存在，请求聚焦" );
        focusRequest(swd);
        return;
    }

    fclose ( fopen ( fileName, "w" ) );

    char h[16];
    char w[16];
    readFromConfig ( "Setting-Window-Height", h );
    readFromConfig ( "Setting-Window-Width", w );

    int height = str2int ( h );
    int width = str2int ( w );

    GtkWidget *window;
    GtkWidget *horizontalBox;
    GtkWidget *menuScrollWindow;
    GtkWidget *contentScrollWindow;
    GtkWidget *listBox;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_keep_above ( GTK_WINDOW(window), TRUE );
    settingWindowData->window = window;
    gtk_window_set_icon ( GTK_WINDOW(window), gdk_pixbuf_new_from_file(
                expanduser("/home/$USER/.stran/tran.png"), 0) );

    gtk_window_set_default_size(GTK_WINDOW(window),
            width == 0 ? 850 : width,
            height == 0 ? 850*0.618 : height );

    GtkWidget *headerBar = gtk_header_bar_new();
    GtkWidget *headerBox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
    GtkWidget *headerLayoutLeft = gtk_layout_new(null, null);

    gtk_header_bar_pack_start ( GTK_HEADER_BAR(headerBar), headerBox );
    gtk_box_pack_start ( GTK_BOX(headerBox), headerLayoutLeft, true, true, true );
    gtk_widget_set_size_request ( headerLayoutLeft, MENU_WINDOW_WIDTH-10, -1 );

    //gtk_header_bar_set_title ( GTK_HEADER_BAR(headerBar), "Settings" );
    gtk_header_bar_set_show_close_button ( GTK_HEADER_BAR(headerBar), true );
    gtk_window_set_titlebar(GTK_WINDOW(window), headerBar);
    GtkWidget *headerLeftLabel = gtk_label_new ( "Settings" );

    GtkWidget *headerSeparator = gtk_separator_new ( GTK_ORIENTATION_VERTICAL );
    gtk_box_pack_start ( GTK_BOX(headerBox), headerSeparator, true, true, true );

    settingWindowData->headerBarLeftLabel = headerLeftLabel;
    settingWindowData->headerLayoutLeft = headerLayoutLeft;

    boldLabel ( headerLeftLabel );

    GtkWidget *searchButton = gtk_button_new();

    gtk_layout_put ( GTK_LAYOUT(headerLayoutLeft), searchButton, 0, 0 );
    gtk_layout_put ( GTK_LAYOUT(headerLayoutLeft), headerLeftLabel, 100, 15 );
    gtk_button_set_image(GTK_BUTTON(searchButton),
            gtk_image_new_from_icon_name ( "edit-find", GTK_ICON_SIZE_BUTTON) );

    horizontalBox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
    menuScrollWindow = gtk_scrolled_window_new ( NULL, NULL );
    gtk_widget_set_size_request ( menuScrollWindow, MENU_WINDOW_WIDTH, -1 );
    contentScrollWindow = gtk_scrolled_window_new ( NULL, NULL );
    settingWindowData->contentScrollWindow = contentScrollWindow;

    listBox = gtk_list_box_new (  );
    settingWindowData->listBox = listBox;
    initSettingItems ( listBox );
    g_signal_connect ( GTK_CONTAINER(listBox), "row-selected",
            G_CALLBACK(on_row_selected_call_back), settingWindowData  );
    GtkWidget *windowSeparator = gtk_separator_new ( GTK_ORIENTATION_VERTICAL );

    /* rootWindow<-box<-scrollWindow<-listBox*/
    gtk_container_add ( GTK_CONTAINER(window), horizontalBox );
    gtk_box_pack_start ( GTK_BOX(horizontalBox), menuScrollWindow, false, false, false );
    gtk_box_pack_start ( GTK_BOX(horizontalBox), windowSeparator, false, false, false );
    gtk_box_pack_start ( GTK_BOX(horizontalBox), contentScrollWindow, false, true, false );
    gtk_container_add ( GTK_CONTAINER(menuScrollWindow), listBox );

    g_signal_connect(G_OBJECT(window), "destroy",
            G_CALLBACK(gtk_window_destroy),settingWindowData);

    g_signal_connect(G_OBJECT(window), "key-press-event",
            G_CALLBACK(on_key_press_cb),settingWindowData);

    g_signal_connect ( GTK_CONTAINER(searchButton), "clicked",
            G_CALLBACK(search_button_click_call_back), settingWindowData );

    settingWindowData->configure_event_signal_id = 
        g_signal_connect ( window, "configure-event",
                G_CALLBACK(on_configure_event_cb), settingWindowData );

    gtk_widget_show_all(window);

    swd->timeoutid = 
        g_timeout_add ( 100, focusProxy, swd);

    /* TEST CODE*/
    /* gtk_list_box_select_row ( GTK_LIST_BOX(listBox),
     * gtk_list_box_get_row_at_index(GTK_LIST_BOX( listBox ), 1) ); */
}
