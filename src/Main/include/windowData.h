#ifndef __WINDOW_DATA__
#define __WINDOW_DATA__

#include "configControl.h"
#include "shmData.h"
#include "shmidData.h"
#include "memory.h"

#define GET_SHMADDR(shmData, who)  ( ( who ) == BING ? (shmData->shmaddr_bing) : \
        ( (who) == MYSQL ? shmData->shmaddr_mysql : shmData->shmaddr_google))

#define GET_BUTTON(win,who)  ( ( who ) == BING ? (WINDATA(win)->baiduButton) :\
        ( (who) == MYSQL ? WINDATA(win)->mysqlButton : WINDATA(win)->googleButton))

#define TYPE(who)     ( ( who ) == BING ? ( ONLINE ) :\
        ( ( who ) == MYSQL ? OFFLINE : -1 ))

#define WHO(addr)     ( ( addr ) == shmaddr_mysql ? MYSQL : \
        (addr == shmaddr_google ? GOOGLE : BING))

typedef struct {

    char audioOnline_en[512];
    char audioOnline_am[512];

    char audioOffline_en[512];
    char audioOffline_am[512];

}AudioData;

typedef struct {

    volatile int pointerx;
    volatile int pointery;
    volatile int previousx;
    volatile int previousy;
    int          buttonState;
    volatile int buttonPress;
    volatile int buttonRelease;
    volatile int startSlide;
    int          recallPreviousFlag;
    int          tranPicAction;

    volatile sig_atomic_t action;
    volatile sig_atomic_t inNewWin;
    volatile sig_atomic_t canNewEntrance;
    volatile sig_atomic_t canNewWin;
    volatile sig_atomic_t destroyIcon;
    volatile sig_atomic_t iconShowing;
    volatile sig_atomic_t sigtermNotify;

}CommunicationData;

typedef struct {

    int argc;
    char **argv;
    ConfigData *cd;
    CommunicationData *md;
    ShmData *sd;
    ShmIdData *sid;
    MemoryData *med;
}Arg ;

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

    Arg *arg;
    ConfigData *cd;
    CommunicationData *md;
    ShmData *sd;
    AudioData *ad;
    MemoryData *med;

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

GtkWidget *newCalibrationButton ( WinData *win );

#endif
