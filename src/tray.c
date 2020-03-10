#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include "settingWindowData.h"
#include "tray.h"
#include "panel.h"

void Settings(GtkWidget *menuItemSetting, SettingWindowData *settingWindowData) {

    settingWindow(menuItemSetting, settingWindowData);
}


void trayIconPopup(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer menu)
{
    //gtk_menu_popup(GTK_MENU(menu), NULL, NULL, gtk_status_icon_position_menu, status_icon, button, activate_time);
    gtk_menu_popup_at_pointer ( GTK_MENU(menu), NULL);
}

gboolean menuPopup (GtkStatusIcon *status_icon, GdkEvent *event, gpointer menu) {

    gtk_menu_popup_at_pointer ( GTK_MENU(menu), NULL);

    return TRUE;
}

void initTrayIcon ( SettingWindowData *settingWindowData ) {

    GtkStatusIcon *trayIcon = gtk_status_icon_new_from_file("/home/rease/.stran/tran.png");
    settingWindowData->trayIcon = (GtkWidget*)trayIcon;

    GtkWidget *menu = gtk_menu_new();
    GtkWidget *menuItemSetting = gtk_menu_item_new_with_label ( "Settings" );
    GtkWidget *menuItemRestart = gtk_menu_item_new_with_label ( "Restart" );
    GtkWidget *menuItemExit = gtk_menu_item_new_with_label ( "Exit" );
    gtk_menu_shell_append ( GTK_MENU_SHELL(menu), menuItemSetting );
    gtk_menu_shell_append ( GTK_MENU_SHELL(menu), menuItemRestart );
    gtk_menu_shell_append ( GTK_MENU_SHELL(menu), menuItemExit );
    gtk_widget_show_all ( menu );

    g_signal_connect ( menuItemSetting, "activate", G_CALLBACK(Settings), settingWindowData );
    g_signal_connect ( menuItemExit, "activate", G_CALLBACK(Exit), settingWindowData );
    g_signal_connect ( menuItemRestart, "activate", G_CALLBACK(Restart), settingWindowData );

    g_signal_connect(GTK_STATUS_ICON (trayIcon), "popup-menu", G_CALLBACK(trayIconPopup), menu);
    g_signal_connect(GTK_STATUS_ICON (trayIcon), "button-release-event", G_CALLBACK(menuPopup), menu);
}
