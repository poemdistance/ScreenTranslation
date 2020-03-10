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


int setting()
{
    removeLockFile();

    static SettingWindowData settingWindowData;

    initSettingWindowData( &settingWindowData );

    gtk_init(NULL, NULL);

    initTrayIcon( &settingWindowData );

    /* settingWindow(NULL, &settingWindowData); */

    gtk_main();

    return settingWindowData.selectedItem;
}

