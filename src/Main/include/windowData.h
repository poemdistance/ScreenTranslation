#ifndef __WINDOW_DATA__
#define __WINDOW_DATA__

#include "configControl.h"

#define GET_SHMADDR(who)  ( ( who ) == BAIDU ? (shmaddr_baidu) : \
        ( (who) == MYSQL ? shmaddr_mysql : shmaddr_google))

#define GET_BUTTON(win,who)  ( ( who ) == BAIDU ? (WINDATA(win)->baiduButton) :\
        ( (who) == MYSQL ? WINDATA(win)->mysqlButton : WINDATA(win)->googleButton))

#define TYPE(who)     ( ( who ) == BAIDU ? ( ONLINE ) :\
        ( ( who ) == MYSQL ? OFFLINE : -1 ))

#define WHO(addr)     ( ( addr ) == shmaddr_mysql ? MYSQL : \
        (addr == shmaddr_google ? GOOGLE : BAIDU))

typedef struct WinData{

    int who;
    int gotOfflineTran;
    int gotBaiduTran;
    int gotGoogleTran;

    GtkWidget *window;

    GtkWidget *baiduButton;
    GtkWidget *mysqlButton;
    GtkWidget *googleButton;
    GtkWidget *switchButton;
    GtkWidget *exitButton;
    GtkWidget *calibrationButton;

    gint width;
    gint height;
    gint headerBarHeight;
    gint headerBarWidth;
    gint previousWidth;
    gint previousHeight;

    gboolean quickSearchFlag;
    gboolean recallPreviousFlag;

    ConfigData *cd;
    gboolean mousePress;;
    gboolean mouseRelease;

    GtkWidget *unselectedButton[4];
    GtkWidget *selectedButton[4];
    GtkWidget *selectedPin;
    GtkWidget *unselectedPin;
    GtkWidget *selectedBing;
    GtkWidget *unselectedBing;
    GtkWidget *selectedGoogle;
    GtkWidget *unselectedGoogle;
    GtkWidget *selectedOffline;
    GtkWidget *unselectedOffline;
    GtkWidget *needToBeHiddenWidget[10];
    GtkWidget *ctrl_grid;
    GtkWidget *headerbar;
    GtkWidget *setting_button;
    GtkWidget *setting_button_bottom;
    GtkWidget *content_listbox;
    GtkWidget *src_label;
    GtkWidget *audio_button_en;
    GtkWidget *audio_button_am;
    GtkWidget *phonetic_en;
    GtkWidget *phonetic_am;
    GtkWidget *item_scroll;
    GtkWidget *item_view;
    GtkWidget *item_label;
    GtkWidget *box;
    GtkWidget *ctrl_listbox;
    GtkWidget *phon_listbox;
    GtkWidget *content_box;
    GtkWidget *src_listbox;
    guint16 tran_max_len;
    GdkRGBA rgba;

    gboolean buttonPress;
    gboolean beginDrag;

    gint offsetX;
    gint offsetY;

    gboolean pinEnable;
    gboolean openSettingWindowAction;

    char *shmaddr_setting;

    gint targetx;
    gint targety;
    gboolean moveWindowNotify;
    gboolean doubleClickAction;
    gboolean showLock;

    GtkWidget *bottomBox;
    GtkWidget *topBox;
    GtkWidget *leftBox;
    GtkWidget *rightBox;
    GtkWidget *headerBox;

    GtkWidget *areaWest;
    GtkWidget *areaEast;
    GtkWidget *areaSouth;
    GtkWidget *areaNorth;
    GtkWidget *areaNorthWest;
    GtkWidget *areaNorthEast;
    GtkWidget *areaSouthEast;
    GtkWidget *areaSouthWest;

    GtkWidget *maximizeButton;
    GtkWidget *minimizeButton;
    GtkWidget *destroyButton;

    GtkWidget *mainBox;

    gint time;

}WinData;

typedef struct {

}CommunicationData;

GtkWidget *newCalibrationButton ( WinData *win );

struct Arg {

    int argc;
    char **argv;
    ConfigData *cd;
};

#endif
