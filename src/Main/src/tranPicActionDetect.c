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

static pid_t child_pid;
static int SIGTERM_SIGNAL = 0;

static void exitNotify() {
    SIGTERM_SIGNAL = 1;
}

void readChild() {

    while ( waitpid ( child_pid, NULL, WNOHANG ) > 0 ) {
        SIGTERM_SIGNAL = 1;
    }
}

int detectTranPicAction () {

    int retpid = -1;
    if ( ( retpid = fork() ) == 0 ) {

        char *const cmd[2] = { "extractPic", (char*)0 };
        if ( execv("/usr/bin/extractPic", cmd) < 0 ){
            pbred ( "Execv error in tranPicActionDetect" );
            perror("errno");
        }
    } 

    char *shmaddr_pic = NULL;
    shared_memory_for_pic ( & shmaddr_pic );

    if ( retpid > 0 ) {


        struct sigaction sa;
        sa.sa_handler = exitNotify;
        sigemptyset ( &sa.sa_mask );
        if ( sigaction ( SIGTERM, &sa, NULL ) != 0 )
            err_exit("Sigaction for SIGTERM failed <tranPicActionDetect.c>");

        if ( sigaction ( SIGINT, &sa, NULL ) != 0 )
            err_exit("Sigaction for SIGTERM failed <tranPicActionDetect.c>");

        sa.sa_handler = readChild;
        sigemptyset ( &sa.sa_mask );
        if ( sigaction ( SIGCHLD, &sa, NULL ) != 0 )
            err_exit("Sigaction for SIGCHLD failed <tranPicActionDetect.c>");

        child_pid = retpid;

        while ( 1 ) {

            if ( shmaddr_pic[1] == SCREEN_SHOT ) {

                printf("启动区域截图\n");
                system("gnome-screenshot -a -B -f /home/$USER/.stran/pic/1.png");
                shmaddr_pic[1] = CLEAR;
                shmaddr_pic[2] = SCREEN_SHOT;
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
