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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <time.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

/*for gdk_x11_window_get_xid()*/
#include <gdk/gdkx.h> 

#include <X11/extensions/XTest.h>

#include "printWithColor.h"
#include "sharedMemory.h"
#include "memoryControl.h"

#define TEXTSIZE (1024*1024)

#define SINGLECLICK (1)
#define DOUBLECLICK (2)
#define SLIDE (3)
#define ALLONE (4)

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


#define QuickSearchShortcutPressed_FLAG (0)
#define CTRL_C_PRESSED_FLAG (1)
#define WINDOW_OPENED_FLAG (2)
#define SEARCH_WINDOW_OPENED_FLAG (4)
#define SELECT_EXCLUDE_FLAG (3)
#define QUICK_SEARCH_NOTIFY (5)
#define RECALL_PREVIOUS_TRAN (6)

void tranSelect();
void checkSelectionChanged();
int isEmpty( char *buf );
int detectTranPicAction ();

#endif
