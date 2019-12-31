#ifndef __WINDOW_DATA__
#define __WINDOW_DATA__

#define GET_SHMADDR(who)  ( ( who ) == BAIDU ? (shmaddr_baidu) : \
        ( (who) == MYSQL ? shmaddr_mysql : shmaddr_google))

#define GET_BUTTON(win,who)  ( ( who ) == BAIDU ? (WINDATA(win)->baiduButton) :\
        ( (who) == MYSQL ? WINDATA(win)->mysqlButton : WINDATA(win)->googleButton))

#define TYPE(who)     ( ( who ) == BAIDU ? ( ONLINE ) :\
        ( ( who ) == MYSQL ? OFFLINE : -1 ))

#define WHO(addr)     ( ( addr ) == shmaddr_mysql ? MYSQL : \
        (addr == shmaddr_google ? GOOGLE : BAIDU))

#define AUDIO(type)  ( ( type ) == ONLINE ? ( url_online ) : ( url_offline ) )


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
    char *audio_online[2];
}Baidu;

Baidu bw;

typedef struct Mysql {
    double width;
    double height;
    int lines;
    int maxlen;
    char *audio_offline[2];
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
    GtkWidget *indicateButton;
    GtkWidget *switchButton;
    GtkWidget *calibrationButton;

    GdkPixbuf *srcBackgroundImage;

    gint width;
    gint height;
    gint lastwidth;
    gint lastheight;
    gint forceResize;

    gint hadRedirect;
    gint hadShowGoogleResult;

    /*used for calibration*/
    gint press;
    gint drag;
    gint enter;
    gint ox, oy; /* current x,y position of button*/
    gdouble cx, cy; /* current x,y position relative to the up left corner of button*/

}WinData;

GtkWidget *newCalibrationButton ( WinData *win );

#endif
