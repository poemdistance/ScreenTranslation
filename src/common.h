#ifndef __COMMON_H__
#define __COMMON_H__

#include <gtk/gtk.h>
#include <gtk/gtkwindow.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <time.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

#define NOTACTIONTEXT ("NOTACTIONTEXT")
#define TEXTSIZE (1024*1024)
#define SINGLECLICK (1)
#define DOUBLECLICK (2)
#define SLIDE (3)
#define EMPTYFLAG ('5')
#define EXITFLAG ('4')
#define NULLCHAR ('3')
#define ERRCHAR ('2')
#define FINFLAG ('1')
#define CLEAR   ('0')
#define ACTUALSTART (10)
#define BAIDUSIZE (6)
#define GOOGLESIZE (3)
#define LINELEN (28)

void show_utf8_prop(Display *dpy, Window w, Atom p, char *text);

int getClipboard(char *text);
void delay();
void writePipe(char *text, int fd);
void handler(int signo);
int isApp(char *appName, char *name);
int previous( int n );
int isAction(int history[], int last, int action);
void quit();
void sync_key( int *fd, struct input_event *event, int *len);
void press(int fd, int keyCode);
void release(int fd, int keyCode);
void simulateKey(int fd,  int key[], int len);
void err_exit(char *buf);
void *GuiEntrance(void *arg);
void *DetectMouse(void *arg);
int shared_memory_for_google_translate(char **addr);
int shared_memory_for_baidu_translate(char **addr);
void *newWindow(void * arg);
void adjustStr(char *p[3], int len, char *storage[3]);
int adjustStrForScrolledWin(int len, char *source);
void notify(int (*history)[4], int *thirdClick, int *releaseButton, int fd[2]);

void * 
sendToClipboard( void *arg );
void separateData(int *index, int len);
void adjustStrForBaidu(int len, char *source, int addSpace, int copy);
int countLines ( int len, char *source );
int countCharNums ( char *source );

struct clickDate {
    GtkWidget *window;
    GtkWidget *button;
};

struct Arg {
    int argc;
    char **argv;
    char *addr_google;
    char *addr_baidu;
};


#define GETEKYDIR ("/tmp")
#define PROJECTID  (2333)
#define PROJECTID2  (2334)
#define SHMSIZE (1024*1024)

#define PhoneticFlag ( shmaddr_baidu[1] - '0' )
#define NumZhTranFlag ( shmaddr_baidu[2] - '0')
#define NumEnTranFlag ( shmaddr_baidu[3] - '0')
#define OtherWordFormFlag ( shmaddr_baidu[4] - '0')
#define NumAudioFlag ( shmaddr_baidu[5] - '0')

#define SourceInput ((char *)( baidu_result[0] ))
#define Phonetic ((char *)( baidu_result[1] ))
#define ZhTrans ((char *)( baidu_result[2] ))
#define EnTrans ((char *)( baidu_result[3] ))
#define OtherWordForm ((char *)( baidu_result[4] ))
#define Audios ((char *)( baidu_result[5] ))

#endif
