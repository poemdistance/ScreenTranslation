#ifndef __NEW_WINDOW__
#define __NEW_WINDOW__

#include "windowData.h"

#define INDEX_SIZE (30)

GtkWidget *syncImageSize ( GtkWidget *window, gint width, gint height, gpointer *data ) ;

GtkWidget *newBaiduButton ( WinData *win );
GtkWidget *newSwitchButton ( WinData *win );
GtkWidget *newOfflineButton ( WinData *win );
GtkWidget *newIndicateButton ( WinData *win );
GtkWidget *newGoogleButton ( WinData *win );

void insertCalibrationButton( WinData *win );

#define WINDATA(addr) ((WinData*)addr)

#define AUDIO_EN(AudioData, type)  ( type == ONLINE ? ( AudioData->audioOnline_en ) : ( AudioData->audioOffline_en ))
#define AUDIO_AM(AudioData, type)  ( type == ONLINE ? ( AudioData->audioOnline_am ) : ( AudioData->audioOffline_am ))

/* #define SHMADDR_TYPE(type) ( type == ONLINE ? ( shmaddr_bing ) : ( shmaddr_mysql )) */
/* #define PhoneticFlag(type) ( type == ONLINE ? ( shmaddr_bing[1] - '0' ) : ( shmaddr_mysql[1] - '0' )) */
/* #define NumZhTranFlag(type) ( type == ONLINE ? ( shmaddr_bing[2] - '0' ) : ( shmaddr_mysql[2] - '0' )) */
/* #define NumEnTranFlag(type) ( type == ONLINE ? ( shmaddr_bing[3] - '0' ) : ( shmaddr_mysql[3] - '0' )) */
/* #define OtherWordFormFlag(type) ( type == ONLINE ? ( shmaddr_bing[4] - '0' ) : ( shmaddr_mysql[4] - '0' )) */
/* #define NumAudioFlag(type) ( type == ONLINE ? ( shmaddr_bing[5] - '0' ) : ( shmaddr_mysql[5] - '0' )) */

#define SourceInput(MemoryData, type) ( type == ONLINE ? (( char *) MemoryData->bing_result[0][0]) : ((char*)MemoryData->mysql_result[0][0]))
/* #define Phonetic(type)( type == ONLINE ? (( char *) bing_result[1][0]) : ((char*)mysql_result[1][0])) */
/* #define ZhTrans(type, i) ( type == ONLINE ? (( char *) bing_result[2][i]) : ((char*)mysql_result[2][i])) */
/* #define EnTrans(type) ( type == ONLINE ? (( char *) bing_result[3][0]) : ((char*)mysql_result[3][0])) */
/* #define OtherWordForm(type) ( type == ONLINE ? (( char *) bing_result[4][0]) : ((char*)mysql_result[4][0])) */
/* #define Audios(type) ( type == ONLINE ? (( char *) bing_result[5][0]) : ((char*)mysql_result[5][0])) */


int destroyNormalWin(GtkWidget *window, WinData *win);
int waitForContinue();
void printDebugInfo();
int  newScrolledWin();
void setFontProperties(GtkTextBuffer *buf, GtkTextIter *iter);
int changeDisplay(GtkWidget *button, WinData *data);
void displayGoogleTrans(GtkWidget *button, gpointer *arg);
void displayBaiduTrans(GtkWidget *button,  void **arg );
void displayOfflineTrans ( GtkWidget *button, gpointer *arg );
void syncScrolledWinWithConfigEvent ( GtkWidget *window, GdkEvent *event, gpointer *wd );
void syncNormalWinForConfigEvent( GtkWidget *window, GdkEvent *event, gpointer scroll );
int adjustWinSize(GtkWidget *button, gpointer *arg, int which);
int setWinSizeForNormalWin ( WinData *window, char *addr, int type);
void showGoogleScrolledWin(GtkTextBuffer *gtbuf, GtkTextIter *iter, WinData *wd);

gboolean on_key_press_cb ( GtkWidget *window, GdkEventKey *event, WinData *wd );

void *newNormalWindow ( void *data );
void adjustStrForGoogle(char *p[3], int len, char *storage[3], int *enterNum);

void separateDataForBaidu(int *index, int len, int type, WinData *wd );
void adjustStrForBaidu(int len, char *source, int addSpace, int copy, int *enterNum);
int getLinesOfGoogleTrans ( int *index_google );
void separateGoogleData ( int *index_google, int len, WinData *wd );
int countCharNums ( char *source );

#endif
