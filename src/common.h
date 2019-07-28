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
void *GuiEntry(void *arg);
void *DetectMouse(void *arg);
int shmCreate(char **addr);
void *newWindow(void * arg);
void adjustStr(char *p[3], int len, char *storage[3]);

struct clickDate {
    GtkWidget *window;
    GtkWidget *button;
};

struct Arg {
    int argc;
    char **argv;
    char *addr;
};


#define GETEKYDIR ("/tmp")
#define PROJECTID  (2333)
#define SHMSIZE (1024*1024)

#endif
