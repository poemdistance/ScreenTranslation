#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include "printWithColor.h"
#include "configControl.h"
#include "settingWindowData.h" /* Must be placed before*/
#include "settingList.h"
#include "focus.h"
#include "tray.h"
#include "panel.h"
#include "gtkLabel.h"
#include "setting.h"
#include "sharedMemory.h"

/* The declaration of fucntions*/
void iconShowTimeSetting( SettingWindowData *settingWindowData  );
void removeLockFile();
void scale_value_changed_call_back ( GtkRange *delayScale, SettingWindowData *settingWindowData );
gchar * getScaleValue( GtkScale *scale, gdouble value, gpointer data);


extern GtkWidget *items[LIST_BOX_ITEMS];


void initSettingWindowData(SettingWindowData *data) {

    memset ( data, '\0', sizeof(SettingWindowData) );
}


void Exit(GtkWidget *exit, SettingWindowData *swd) {

    swd->selectedItem = 1;
    removeLockFile();
    gtk_main_quit();
}

void Restart(GtkWidget *restart, SettingWindowData *swd) {

    swd->selectedItem = 2;
    removeLockFile();
    gtk_widget_destroy ( swd->trayIcon );
    gtk_main_quit();
}

gboolean check_open_window_shortcut_event ( void *data ) {

    SettingWindowData *swd = data;
    gboolean event_come = FALSE;

    if ( swd->shm == NULL )
        shared_memory_for_setting ( &swd->shm );

    if ( swd->shm[0] == '1' ) {
        event_come = TRUE;
        swd->shm[0] = '0';
        swd->shm[1] = '1'; /* 自鎖標志*/
    }

    if ( event_come ) settingWindow(NULL, swd);

    /* Continue listening the shortcut press event*/
    return TRUE;
}


int setting()
{
    removeLockFile();

    static SettingWindowData settingWindowData;

    initSettingWindowData( &settingWindowData );

    char *shm = NULL;
    shared_memory_for_setting ( &shm );
    settingWindowData.shm = shm;
    memset ( shm, '0', 100 );


    gtk_init(NULL, NULL);

    initTrayIcon( &settingWindowData );

    /* settingWindow(NULL, &settingWindowData); */

    g_timeout_add ( 100, 
            check_open_window_shortcut_event,
            (void*) &settingWindowData );

    gtk_main();

    return settingWindowData.selectedItem;
}

