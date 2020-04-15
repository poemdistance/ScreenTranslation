#ifndef __WINDOW_DATA__
#define __WINDOW_DATA__

#define MENU_WINDOW_WIDTH ( 270 )

#define LIST_BOX_ITEMS ( 60 )
#define null NULL
#define DIFF_LEN (50)
#define true TRUE
#define false FALSE

#define MARGIN_START (50)
#define MARGIN_END (50)
#define MARGIN_TOP (50)
#define MARGIN_BOTTOM (50)

#define LISTBOXROW_MARGIN_START ( 30 )
#define LISTBOXROW_MARGIN_END ( 30 )

#define LISTBOX_ROW_HEIGHT ( 50 )

extern char settingItems[][128] ;
extern char settingItemsIcon[][128];

typedef struct {

    GtkWidget *listBox;
    GtkWidget *box;

}WinPrefSettingWindowData ;

typedef struct WinPosSettingWindowData {

    GtkWidget *listBox;
    gint listBoxHeight;
    GtkWidget *box;
    gchar *prefName;
    GtkWidget *imageHasTitleBar;
    GtkWidget *imageNoTitleBar;
    GtkWidget *currentImage;
    GtkWidget *layout;
    GtkWidget *saveButton;

    gint layoutWidth;
    gint layoutHeight;
    
    GtkWidget *pointer;
    gint pointerOffsetX;
    gint pointerOffsetY;
    gint pointerAbsoluteX;
    gint pointerAbsoluteY;
    gint cx, cy;

    gboolean pointerEnter;
    /* gboolean pointerLeaver; */
    gboolean pointerPress;
    gboolean pointerDrag;

    gint timeoutID_adjustLayoutWidget;
    gint timeoutID_updateLayout;
    gint come;

}WinPosSettingWindowData;

typedef struct IconPositionSettingWindowData {

    GtkWidget *layout;
    GtkWidget *entry;
    GtkWidget *entry1;
    GtkWidget *entry2;
    GtkWidget *pointer;
    GtkWidget *icon;
    GtkWidget *saveButton;

    gboolean iconEnter;
    gboolean iconClick;
    gboolean iconLeave;
    gboolean iconPress;
    gboolean iconRelease;
    gboolean iconDrag;

    gint iconOffsetX;
    gint iconOffsetY;
    gint cx, cy;
    gint come;
    gint timeoutID;

}IconPositionSettingWindowData;

typedef struct DelaySettingWindowData {

    GtkWidget *listBox;
    GtkWidget *boxInSetToDefaultGirdRight;
    gboolean hasUpdateSetToDefaultButton;
    GtkWidget *delayScale;
    gint      scaleValue;

}DelaySettingWindowData;

typedef struct ShortcutSettingWindowData {

    GtkWidget *listBox;
    GdkDevice *grab_pointer;
    GtkWidget *dialog;
    GtkWidget *addButton;
    GtkWidget *cancelButton;
    GtkWidget *removeButton;
    GtkWidget *setButton;
    GtkWidget *replaceButton;
    GtkWidget *image;
    GtkWidget *tipLabel;

    gchar receiveShortcut[64];
    GtkListBoxRow *selectedRow;

}ShortcutSettingWindowData;


typedef struct SettingWindowData {

    GtkWidget *window;
    GtkWidget *trayIcon;
    GtkWidget *contentScrollWindow;
    GtkWidget *searchEntry;
    GtkWidget *listBox;
    GtkWidget *searchEntryBox;
    GtkWidget *button;
    GtkWidget *headerBarLeftLabel;
    GtkWidget *headerLayoutLeft;

    gboolean searchEntryShow;

    DelaySettingWindowData *delaySettingWindowData;
    ShortcutSettingWindowData *shortcutSettingWindowData ;
    IconPositionSettingWindowData *iconPositionSettingWindowData;
    WinPosSettingWindowData *winPosSettingWindowData;
    WinPrefSettingWindowData *winPrefSettingWindowData;

    gint width;
    gint height;
    gint previousWidth;
    gint previousHeight;

    gulong configure_event_signal_id;

    gint selectedItem;

    gchar *shm;

    gint timeoutid;

}SettingWindowData;


#endif
