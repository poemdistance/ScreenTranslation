#include <X11/Xlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <signal.h>

#include "common.h"

#define TIMEOUT ( 700 )

pid_t tranPicActionDetect_pid = 0;
pid_t child_pid;
Display *display = NULL;

extern char *shmaddr_keyboard;

void suicide_tranpic() {

    /* 小心kill掉0进程*/
    for ( int i=5; i>0 && tranPicActionDetect_pid != 0; i-- ) {

        kill ( child_pid, SIGKILL );
        while ( waitpid ( child_pid, NULL, WNOHANG ) > 0 );
        usleep(200000);
        kill ( tranPicActionDetect_pid, SIGKILL );
    }
}

void readChild() {

    while ( waitpid ( child_pid, NULL, WNOHANG ) > 0 );
}

int detectTranPicAction () {

    printf("In detectTranPicAction\n");

    int count = 0;
    int canShot = 0;
    int lock = 1;
    int drop = 0;
    int history_x = 0;
    int history_y = 0;
    int check_x = 0;
    int check_y = 0;
    unsigned int mask; 
    int root_x = -1, root_y = -1; 
    Window root_window; 

    struct timeval now;
    double time1 = 0, lastTime = 0;

    display = XOpenDisplay(NULL);

    struct sigaction sa;
    sa.sa_handler = suicide_tranpic;
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGTERM, &sa, NULL ) != 0 )
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


            XQueryPointer(display, DefaultRootWindow(display), &root_window,\
                    &root_window, &root_x, &root_y, &root_x, &root_y, &mask); 

            gettimeofday ( &now, NULL );
            time1 = ( now.tv_usec + now.tv_sec*1000000 ) / 1000; /*  Unit:ms*/
            if ( drop || (abs ( time1 - lastTime ) > TIMEOUT) )  {
                count = 0;
                lastTime = 0;
                history_x = -100;
                history_y = -100;
                drop = 0;
            }


            /* mask>0保证有按键按下才进入执行逻辑
             * lock保证按键按下后此处逻辑不会被反复执行，直到按键释放lock=1
             * shmaddr_keyboard 保证在窗口打开的情况下不执行if内部逻辑*/
            if ( mask > 0 && lock && mask ==  Button1MotionMask \
                    && shmaddr_keyboard[4] != '1' \
                    && shmaddr_keyboard[2] != '1')
            {


                if ( (history_x != root_x || history_y != root_y) ) {

                    gettimeofday ( &now, NULL );
                    time1 = ( now.tv_usec + now.tv_sec*1000000 ) / 1000;  /* Unit:ms*/

                    /* lastTime==0时, 此次点击作第一次点击看待,去else执行, 否则进if*/
                    if ( lastTime > 0 )
                    {
                        if ( check_x && abs(root_x-history_x) < 21 ) {

                            printf("第3次点击x距离依旧过小，抛弃\n");
                            drop = 1;
                            check_x = 0;
                            continue;
                        }

                        if ( check_y && abs(root_y-history_y) < 21 ) {

                            printf("第3次点击y距离依旧过小，抛弃\n");
                            drop = 1;
                            check_y = 0;
                            continue;
                        }

                        if ( abs(root_x-history_x) < 21 && count == 1 ) {
                            printf("x距离过小-1\n");
                            check_x = 1;
                        }

                        if ( abs(root_y-history_y) < 21 && count == 1 ) {

                            printf("y距离过小-1\n");
                            check_y = 1;
                        }


                        if ( abs ( time1 - lastTime ) > TIMEOUT ) {
                            printf("超时抛弃\n");
                            drop = 1;
                            continue;
                        }
                        else if ( ++count == 3 )
                            canShot = 1;
                    }
                    else 
                        ++count;/* 因此次点击作为第一次点击，所以计数值count加1*/

                    /* 更新`上一次点击`的时间戳*/
                    lastTime = time1;

                }
                else 
                    count = 0;/* 出现原地双击，丢弃点击次数统计*/

                /* 更新历史坐标*/
                history_y = root_y;
                history_x = root_x;

                /* 自锁*/
                lock = 0;
            } 

            /* 按键释放，解锁*/
            if ( mask == 0 )
                lock = 1;

            /* canShot==1, 进行截图操作*/
            //if ( (mask == 0 && canShot ) || shmaddr_pic[1] == SCREEN_SHOT ) {
            if ( shmaddr_pic[1] == SCREEN_SHOT ) {

                count = 0;
                check_x = 0;
                check_y = 0;
                canShot = 0;
                shmaddr_pic[1] = CLEAR;
                printf("启动区域截图\n");
                system("gnome-screenshot -a -B -f /home/$USER/.stran/1.png");
            }

            /* 休眠, 防止一直占用CPU*/
            usleep(10000);
        }
    }

    return 0;
}
