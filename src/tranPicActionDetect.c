#include <X11/Xlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <signal.h>

#include "common.h"
#include "cleanup.h"

#define TIMEOUT ( 700 )

pid_t tranPicActionDetect_pid = 0;
pid_t child_pid;
Display *display = NULL;

extern char *shmaddr_keyboard;

static int SIGTERM_SIGNAL = 0;

static void exitNotify() {
    SIGTERM_SIGNAL = 1;
}

void readChild() {

    while ( waitpid ( child_pid, NULL, WNOHANG ) > 0 );
}

int detectTranPicAction () {

    unsigned int mask; 
    int root_x = -1, root_y = -1; 
    Window root_window; 

    display = XOpenDisplay(NULL);

    struct sigaction sa;
    sa.sa_handler = exitNotify;
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGTERM, &sa, NULL ) != 0 )
        err_exit("Sigaction for SIGTERM failed <tranPicActionDetect.c>");
    if ( sigaction ( SIGINT, &sa, NULL ) != 0 )
        err_exit("Sigaction for SIGTERM failed <tranPicActionDetect.c>");

    tranPicActionDetect_pid = getpid();

    int retpid = -1;
    if ( ( retpid  = fork() ) == 0 ) {

        char *const cmd[2] = { "extractPic", (char*)0 };
        if ( execv("/usr/bin/extractPic", cmd) < 0 )
            err_exit("execv extractPic error");
    }

    char *shmaddr_pic = NULL;
    shared_memory_for_pic ( & shmaddr_pic );

    if ( retpid > 0 ) {

        sa.sa_handler = readChild;
        sigemptyset ( &sa.sa_mask );
        if ( sigaction ( SIGCHLD, &sa, NULL ) != 0 )
            err_exit("Sigaction for SIGCHLD failed <tranPicActionDetect.c>");

        child_pid = retpid;

        while ( 1 ) {

            /* 获取指针坐标, 暂时未使用*/
            XQueryPointer(display, DefaultRootWindow(display), &root_window,\
                    &root_window, &root_x, &root_y, &root_x, &root_y, &mask); 


            if ( shmaddr_pic[1] == SCREEN_SHOT ) {

                printf("启动区域截图\n");
                system("gnome-screenshot -a -B -f /home/$USER/.stran/pic/1.png");
                shmaddr_pic[1] = CLEAR;
            }

            if ( SIGTERM_SIGNAL ) break;

            /* 休眠, 防止一直占用CPU*/
            usleep(100000);

        } /* while loop*/


        /* 关闭extrancPic子进程*/
        kill ( child_pid, SIGKILL );
        while ( waitpid ( child_pid, NULL, WNOHANG ) > 0 );
    }

    pbcyan ( "TranPic 退出" );

    return 0;
}
