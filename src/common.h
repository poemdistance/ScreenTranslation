#ifndef __COMMON_H__
#define __COMMON_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
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
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <time.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <bsd/unistd.h>
#include <sys/types.h>

/*for gdk_x11_window_get_xid()*/
#include <gdk/gdkx.h> 

#include <X11/extensions/XTest.h>

#include "printWithColor.h"
#include "sharedMemory.h"
#include "memoryControl.h"

#define TEXTSIZE (1024*1024)

enum {
    SINGLE_CLICK=1,
    DOUBLE_CLICK,
    SLIDE,
    ALL_ONE,
    BUTTON_PRESS,
    BUTTON_RELEASE,
    BUTTON_NO_ACTION,
};

#define EMPTYFLAG ('5')
#define EXITFLAG ('4')
#define NULLCHAR ('3')
#define ERRCHAR ('2')
#define FINFLAG ('1')
#define CLEAR   ('0')

#define SCREEN_SHOT ('1')

#define ACTUALSTART (10)

#define BAIDUSIZE (6)
#define GOOGLESIZE (3)
#define MYSQLSIZE (BAIDUSIZE)
#define LINE_LEN (28)

#define BOTTOM_OFFSET ( 45 )
#define RIGHT_BORDER_OFFSET ( 50 )
#define INDICATE_OFFSET ( 80 )

#define BAIDU (3)
#define GOOGLE (2)
#define MYSQL (1)

#define OFFLINE (1)
#define ONLINE (2)


enum {
    QUICK_SEARCH_FLAG,
    CTRL_C_PRESSED_FLAG,
    WINDOW_OPENED_FLAG,
    SEARCH_WINDOW_OPENED_FLAG,
    SELECT_EXCLUDE_FLAG,
    QUICK_SEARCH_NOTIFY,
    RECALL_PREVIOUS_TRAN
};

void tranSelect();
void checkSelectionChanged();
int isEmpty( char *buf );
int detectTranPicAction ();

#endif
