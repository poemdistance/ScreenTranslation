#ifndef __TRAY_ICON__
#define __TRAY_ICON__


void trayIconPopup(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer menu);

gboolean menuPopup (GtkStatusIcon *status_icon, GdkEvent *event, gpointer menu);
void initTrayIcon ( SettingWindowData *settingWindowData );

#endif
