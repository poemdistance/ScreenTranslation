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

#define STORE_DISPLAY_LINES_NUM(win, who, value) \
    if ( who == BAIDU ) { \
        WINDATA(win)->bw->lines = value;\
    }\
    else if ( who == MYSQL ){\
        WINDATA(win)->mw->lines = value;\
    } \
    else if ( who == GOOGLE ) {\
        WINDATA(win)->gw->lines = value;\
    }

#define STORE_DISPLAY_MAX_LEN(win, who, value) \
    if ( who == BAIDU ) { \
        WINDATA(win)->bw->maxlen = value;\
    }\
    else if ( who == MYSQL ){\
        WINDATA(win)->mw->maxlen = value;\
    } \
    else if ( who == GOOGLE ) {\
        WINDATA(win)->gw->maxlen = value;\
    }

#define GET_DISPLAY_MAX_LEN(win, who) ( who == BAIDU ? WINDATA(win)->bw->maxlen : \
        ( who == GOOGLE ? WINDATA(win)->gw->maxlen : WINDATA(win)->mw->maxlen ))

#define GET_DISPLAY_LINES_NUM(win, who) ( who == BAIDU ? WINDATA(win)->bw->lines : \
        ( who == GOOGLE ? WINDATA(win)->gw->lines : WINDATA(win)->mw->lines ))

#define STORE_DISPLAY_WIDTH(who,value) \
    if(who==BAIDU) {\
        bw.width = value;\
    }\
    else if ( who == MYSQL ){\
        mw.width = value;\
    }\
    else if (who == GOOGLE ){\
        gw.width = value;\
    }

#define STORE_DISPLAY_HEIGHT(who,value) \
    if(who==BAIDU) {\
        bw.height=value;\
    }\
    else if ( who == MYSQL ){\
        mw.height = value;\
    }\
    else if (who == GOOGLE ){\
        gw.height = value;\
    }

#define GET_DISPLAY_WIDTH(who)    ( ( who ) == BAIDU ? ( bw.width ): \
        ( (who) == GOOGLE ? ( gw.width ) : ( mw.width ) ) )

#define GET_DISPLAY_HEIGHT(who)   ( ( who ) == BAIDU ? ( bw.height ): \
        ( (who) == GOOGLE ? ( gw.height ) : ( mw.height ) ) )

typedef struct Google {
    double width;
    double height;
    int lines;
    int maxlen;
}Google;

Google gw;

typedef struct Baidu {
    double width;
    double height;
    int lines;
    int maxlen;
}Baidu;

Baidu bw;

typedef struct Mysql {
    double width;
    double height;
    int lines;
    int maxlen;
}Mysql;

Mysql mw;

typedef struct AudioButton {

    gint press;
    gint drag;
    gint enter;
    gint ox, oy; /* current x,y position of button*/
    gdouble cx, cy; /* current x,y position relative to the up left corner of button*/

}AudioButton;

typedef struct WinData{

    int who;
    int gotOfflineTran;
    int gotBaiduTran;
    int gotGoogleTran;
    int specific;

    Baidu *bw;
    Mysql *mw;
    Google *gw;

    GtkWidget *window;
    GtkWidget *layout;
    GtkWidget *view;

    GdkWindow *gdkwin;

    GtkTextIter *iter;
    GtkTextBuffer *buf;

    GtkWidget *audio;
    GtkWidget *scroll;

    GtkWidget *image;
    GtkWidget *oldImage;

    GtkWidget *baiduButton;
    GtkWidget *mysqlButton;
    GtkWidget *googleButton;
    GtkWidget *switchButton;
    GtkWidget *exitButton;
    GtkWidget *calibrationButton;

    GdkPixbuf *srcBackgroundImage;

    gint width;
    gint height;
    gint headerBarHeight;
    gint headerBarWidth;
    gint previousWidth;
    gint previousHeight;

    gint hadRedirect;
    gint hadShowGoogleResult;

    gboolean quickSearchFlag;

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

GtkWidget *newCalibrationButton ( WinData *win );

struct Arg {

    int argc;
    char **argv;
    char *addr_google;
    char *addr_baidu;
    ConfigData *cd;
};

#endif
