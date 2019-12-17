#ifndef __WINDOW_DATA__
#define __WINDOW_DATA__

typedef struct Google {
    double width;
    double height;
}Google;

Google gw;

typedef struct Baidu {
    double width;
    double height;
    char *audio_online[2];
    char *audio_offline[2];
}Baidu;

Baidu bw;

typedef struct Mysql {
    double width;
    double height;
    char *audio_online[2];
    char *audio_offline[2];
}Mysql;

Mysql mw;

typedef struct WinData{

    int who;
    int getOfflineTranslation;
    int specific;

    GtkWidget *window;
    GtkWidget *layout;
    GtkWidget *view;

    GdkWindow *gdkwin;

    GtkTextIter *iter;
    GtkTextBuffer *buf;

    GtkWidget *volume;
    GtkWidget *scroll;

    GtkWidget *image;
    GtkWidget *oldImage;

    GtkWidget *baiduButton;
    GtkWidget *offlineButton;
    GtkWidget *googleButton;
    GtkWidget *indicateButton;
    GtkWidget *switchButton;

    GdkPixbuf *srcBackgroundImage;

    gint width;
    gint height;
    gint lastwidth;
    gint lastheight;
    gint forceResize;

    gint lineHeight;
    gint phonPos;

    gint hadRedirect;

    gint hadShowGoogleResult;

    int index_google[2];
    char **storage;

    int switchEvent;

}WinData;

#endif
