#ifndef __NEW_WINDOW__
#define __NEW_WINDOW__

#include "windowData.h"

GtkWidget *syncImageSize ( GtkWidget *window, gint width, gint height, gpointer *data ) ;

GtkWidget *newBaiduButton ( WinData *win );
GtkWidget *newSwitchButton ( WinData *win );
GtkWidget *newOfflineButton ( WinData *win );
GtkWidget *newIndicateButton ( WinData *win );
GtkWidget *newGoogleButton ( WinData *win );

extern char *text;


extern char *baidu_result[BAIDUSIZE];
extern char *google_result[GOOGLESIZE];
extern char *mysql_result[MYSQLSIZE];

extern int InNewWin;
extern char *shmaddr_google;
extern char *shmaddr_baidu;
extern char *shmaddr_keyboard;
extern char *shmaddr_mysql;
extern char *shmaddr_pic;

#define WINDATA(addr) ((WinData*)addr)

#define audio_en(type)  ( type == ONLINE ? ( audioOnline_en ) : ( audioOffline_en ))
#define audio_uk(type)  ( type == ONLINE ? ( audioOnline_uk ) : ( audioOffline_uk ))

#define shmaddr_type(type) ( type == ONLINE ? ( shmaddr_baidu ) : ( shmaddr_mysql ))
#define PhoneticFlag(type) ( type == ONLINE ? ( shmaddr_baidu[1] - '0' ) : ( shmaddr_mysql[1] - '0' ))
#define NumZhTranFlag(type) ( type == ONLINE ? ( shmaddr_baidu[2] - '0' ) : ( shmaddr_mysql[2] - '0' ))
#define NumEnTranFlag(type) ( type == ONLINE ? ( shmaddr_baidu[3] - '0' ) : ( shmaddr_mysql[3] - '0' ))
#define OtherWordFormFlag(type) ( type == ONLINE ? ( shmaddr_baidu[4] - '0' ) : ( shmaddr_mysql[4] - '0' ))
#define NumAudioFlag(type) ( type == ONLINE ? ( shmaddr_baidu[5] - '0' ) : ( shmaddr_mysql[5] - '0' ))

#define SourceInput(type) ( type == ONLINE ? (( char *) baidu_result[0]) : ((char*)mysql_result[0]))
#define Phonetic(type)( type == ONLINE ? (( char *) baidu_result[1]) : ((char*)mysql_result[1]))
#define ZhTrans(type) ( type == ONLINE ? (( char *) baidu_result[2]) : ((char*)mysql_result[2]))
#define EnTrans(type) ( type == ONLINE ? (( char *) baidu_result[3]) : ((char*)mysql_result[3]))
#define OtherWordForm(type) ( type == ONLINE ? (( char *) baidu_result[4]) : ((char*)mysql_result[4]))
#define Audios(type) ( type == ONLINE ? (( char *) baidu_result[5]) : ((char*)mysql_result[5]))


int destroyNormalWin(GtkWidget *window, WinData *win);
int waitForContinue();
void getIndex(int *index, char *addr);
void printDebugInfo();
int  newScrolledWin();
void setFontProperties(GtkTextBuffer *buf, GtkTextIter *iter);
void changeDisplay(GtkWidget *button, gpointer *arg);
void displayGoogleTrans(GtkWidget *button, gpointer *arg);
void displayBaiduTrans(GtkWidget *button,  void **arg );
void displayOfflineTrans ( GtkWidget *button, gpointer *arg );
void syncScrolledWinWithConfigEvent ( GtkWidget *window, GdkEvent *event, gpointer *wd );
void syncNormalWinForConfigEvent( GtkWidget *window, GdkEvent *event, gpointer scroll );
void adjustWinSize(GtkWidget *button, gpointer *arg, int which);
void setWinSizeForNormalWin ( WinData *window, char *addr, int type);
void showGoogleScrolledWin(GtkTextBuffer *gtbuf, GtkTextIter *iter, WinData *wd);
int getMaxLenOfBaiduTrans() ;
int getLinesOfBaiduTrans () ;

gboolean key_press ( GtkWidget *window, GdkEventKey *event, gpointer *data );

void *newNormalWindow();
void adjustStr(char *p[3], int len, char *storage[3]);

void separateDataForBaidu(int *index, int len, int type);
void adjustStrForBaidu(int len, char *source, int addSpace, int copy);
int getLinesOfGoogleTrans ( int *index_google );
void separateGoogleDataSetWinSize ( int *index_google );
int countLines ( int len, char *source );
int countCharNums ( char *source );
int adjustStrForScrolledWin(int len, char *source);

#endif
