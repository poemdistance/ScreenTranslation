#ifndef __SETTING_LIST__
#define __SETTING_LIST__

typedef void (*ContentDisplayFunc)(SettingWindowData *settingWindowData);

int getListBoxItemsNum();

int insertListBoxItem ( GtkWidget *child, int position );

int dropListBoxItem ( int position );

int getListBoxItemIndex( GtkWidget *item , SettingWindowData *settingWindowData);

void initSettingItems ( GtkWidget *parent );
#endif
