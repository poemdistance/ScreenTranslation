#ifndef __PANEL__
#define __PANEL__


void remove_widget ( GtkWidget *widget, void *container );

void Settings(GtkWidget *menuItemSetting, SettingWindowData *settingWindowData);

void Exit(GtkWidget *exit, SettingWindowData *swd);

void Restart(GtkWidget *restart, SettingWindowData *swd);

void settingWindow(GtkWidget  *menuItemSetting, SettingWindowData *settingWindowData);

void iconShowTimeSetting( SettingWindowData *settingWindowData  );

void shortcutSetting(  SettingWindowData *settingWindowData   );

void iconPositionSetting ( SettingWindowData *settingWindow );

void windowPosSetting ( SettingWindowData *settingWindow );

void windowPrefSetting ( SettingWindowData *swd );

void mouseActionSetting ( SettingWindowData *swd );


gboolean on_configure_event_cb ( 
        GtkWindow *window,
        GdkEvent *event,
        SettingWindowData *settingWindowData );

typedef gboolean (*Super)(
        GtkWindow *window,
        GdkEvent *event,
        SettingWindowData *settingWindowData );

#endif
