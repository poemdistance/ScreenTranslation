/* 
 * 程序功能:
 *
 * 监控stran翻译程序, 若发生异常导致退出
 * 重新启动之.*/

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "printWithColor.h"
#include "setting.h"
#include "expanduser.h"

#define EXIT (1)
#define RESTART (2)

static volatile sig_atomic_t PROCESS_EXIT_FLAG = 0;
static volatile sig_atomic_t SIGTERM_NOTIFY = 0;

static int RESTART_SIGNAL = 0;
static pid_t needWait = 0;
static pid_t pid_mstran = 0;
static pid_t pid_stran = 0;
static pid_t cp_mstran = 0;;

static void handler()  { 

    pid_t pid_stran = 0;

    while ( ( pid_stran = waitpid (needWait, NULL, WNOHANG) ) > 0 ) {
        /* 其实这里可以不用加if，因为如果不是这个程序退出返回0不会进入这里*/
        if ( pid_stran == needWait )
            PROCESS_EXIT_FLAG = 1;
    }
}

void readChild ( ) {

    while ( waitpid (cp_mstran, NULL, WNOHANG) > 0 );
}

void restart (  ) {
    RESTART_SIGNAL = 1;
}

int itoa ( int n, char *buf ) {

    return sprintf(buf, "%d", n);
}

void sigterm() {
    SIGTERM_NOTIFY = 1;
}

    int
main(int argc, char **argv)
{
    struct sigaction sa;
    /* pid_t pid_stran = -1; */
    /* pid_t pid_mstran = -1; */
    int ret;

    pyellow("Monitor Process %d", getpid());

    pid_mstran = fork();

    /* 主进程*/
    if ( pid_mstran > 0 ) {

        cp_mstran = pid_mstran;

        sigemptyset ( &sa.sa_mask );
        sa.sa_flags = SA_RESTART;
        sa.sa_handler = readChild;;
        if ( sigaction ( SIGCHLD, &sa, NULL ) == -1) {
            pbred("sigaction failed to exec (SIGCHLD)");
            exit(1);
        }
        sa.sa_handler = sigterm;
        if ( sigaction ( SIGTERM, &sa, NULL ) == -1) {
            pbred("sigaction failed to exec (SIGCHLD)");
            exit(1);
        }

        while ( 1 ) {
            ret = setting();
            if ( ret == EXIT ) break;
            if ( ret == RESTART ) {
                pmag ( "接收到重启信号" );
                kill ( pid_mstran, SIGTERM );
                sleep(3);
                execv ( argv[0], argv );
            }
        }
        pmag ( "退出监控程序主进程" );
        kill ( pid_mstran, SIGTERM );
        pid_stran = -1;
        pid_mstran = -1;
        needWait = -1;
    }

    if ( pid_mstran == 0 )
        /* 孙子进程*/
        pid_stran = fork();

    /* 子进程：
     *
     * 监控子程序状态，如果子程序退出，则再fork一个子进程重新启动之*/
    if ( pid_mstran == 0 && pid_stran > 0 ) {

        sa.sa_handler = handler;;
        sigemptyset ( &sa.sa_mask );
        sa.sa_flags = SA_RESTART;
        if ( sigaction ( SIGCHLD, &sa, NULL ) == -1) {
            pbred("sigaction failed to exec (SIGCHLD)");
            exit(1);
        }

        sa.sa_handler = sigterm; 
        if ( sigaction ( SIGINT, &sa, NULL ) == -1) {
            pbred("sigaction failed to exec (SIGINT)");
            exit(1);
        }
        if ( sigaction ( SIGTERM, &sa, NULL ) == -1) {
            pbred("sigaction failed to exec (SIGTERM)");
            exit(1);
        }

        needWait = pid_stran;
        while ( 1 ) { 
            usleep ( 100000 );
            if ( PROCESS_EXIT_FLAG == 1 ) {
                pmag("stran 子进程已退出, 准备重新启动");

                PROCESS_EXIT_FLAG = 0;
                kill ( -needWait, SIGKILL );
                pid_stran = fork();

                if ( pid_stran > 0 )
                    needWait = pid_stran;

                /* 子进程需要跳出while循环进入下面的翻译程序启动代码*/
                if ( pid_stran == 0 ) break;
            }

            if ( SIGTERM_NOTIFY ) {
                /* 清理进程组*/
                kill( -needWait, SIGTERM );
                break;
            }
        }
    }

    if ( pid_stran == 0 ) {

        if (execl ( "/usr/bin/stran", "stran", NULL ) == -1) {
            pbred("execl error occured");
            perror("execl errno");
            exit(1);
        }
    }

    /* Prevent no enough sleep time because of the interruption*/
    while ( usleep(500000) != 0 );

    /* Make sure all processes are cleaned up*/
    if ( needWait != -1 )
        kill ( -needWait, SIGKILL );

    pbmag ( "监控程序准备退出: %d , needWait=%d", getpid(), needWait );

    return 0;
}

