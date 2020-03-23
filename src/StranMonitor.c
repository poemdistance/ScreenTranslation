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

int PROCESS_EXIT_FLAG = 0;
int CTRL_C_FLAG = 0;
volatile sig_atomic_t SIGTERM_NOTIFY = 0;
static int RESTART_SIGNAL = 0;

pid_t needWait = 0;
pid_t pid_mstran = 0;
pid_t pid_stran = 0;

static void handler()  { 

    pid_t pid_stran = 0;

    while ( ( pid_stran = waitpid (needWait, NULL, WNOHANG) ) > 0 ) {
        /* 其实这里可以不用加if，因为如果不是这个程序退出返回0不会进入这里*/
        if ( pid_stran == needWait )
            PROCESS_EXIT_FLAG = 1;
    }
}

void readChild ( ) {

    while ( waitpid (pid_mstran, NULL, WNOHANG) > 0 );
}

void restart (  ) {
    RESTART_SIGNAL = 1;
}

int itoa ( int n, char *buf ) {

    return sprintf(buf, "%d", n);
}

void ctrl_c() {

    CTRL_C_FLAG = 1;


    /* kill 15*/
    kill( needWait, SIGTERM );

    usleep(1000000);
    exit(0);
}

void sigterm() {

    SIGTERM_NOTIFY = 1;
    kill( needWait, SIGTERM );
    usleep(1000000);
    exit(0);
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

        pbblue ( "我是父进程:%d", getpid() );

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

        pmag("监控程序正在运行,子进程为:%d", pid_mstran);
        while ( 1 ) {
            ret = setting();
            if ( ret == EXIT ) break;
            if ( ret == RESTART ) {
                /* kill ( pid_mstran, SIGUSR1 ); */
                /* system ( expanduser("/home/$USER/.stran/startup.sh") ); */
                pmag ( "接收到重启信号" );
                kill ( pid_mstran, SIGTERM );
                sleep(3);
                execv ( argv[0], argv );
            }
        }
        kill ( pid_mstran, SIGTERM );
        pid_stran = -1;
        pid_mstran = -1;
    }

    if ( pid_mstran == 0 )
        pid_stran = fork();

    /* 子进程：
     *
     * 监控子程序状态，如果子程序退出，则再fork一个子进程重新启动之*/
    if ( pid_mstran == 0 && pid_stran > 0 ) {

        pbblue ( "我是监控进程:%d", getpid() );

        sa.sa_handler = handler;;
        sigemptyset ( &sa.sa_mask );
        sa.sa_flags = SA_RESTART;
        if ( sigaction ( SIGCHLD, &sa, NULL ) == -1) {
            pbred("sigaction failed to exec (SIGCHLD)");
            exit(1);
        }

        sa.sa_handler = ctrl_c; 
        if ( sigaction ( SIGINT, &sa, NULL ) == -1) {
            pbred("sigaction failed to exec (SIGINT)");
            exit(1);
        }

        sa.sa_handler = sigterm; 
        if ( sigaction ( SIGTERM, &sa, NULL ) == -1) {
            pbred("sigaction failed to exec (SIGTERM)");
            exit(1);
        }

        needWait = pid_stran;
        while ( 1 ) { 
            usleep ( 100000 );
            if ( PROCESS_EXIT_FLAG == 1 ) {
                pmag("stran 子进程已退出, 准备重新启动");

                /* 清理残余进程,(防止异常退出导致有残留进程)*/
                /* system ( "/usr/bin/stoptran" ); */

                PROCESS_EXIT_FLAG = 0;
                kill ( -needWait, SIGKILL );
                pid_stran = fork();

                if ( pid_stran > 0 )
                    needWait = pid_stran;

                /* 子进程需要跳出while循环进入下面的翻译程序启动代码*/
                if ( pid_stran == 0 )
                    break;

            }

            if ( CTRL_C_FLAG == 1 || SIGTERM_NOTIFY) {
                kill( needWait, SIGTERM );
                break;
            }
        }
    }

    if ( pid_stran == 0 ) {

        pbblue ( "我是目标程序主进程:%d", getpid() );

        if (execl ( "/usr/bin/stran", "stran", NULL ) == -1) {
            pbred("execl error occured");
            perror("execl errno");
            exit(1);
        }
    }

    return 0;
}

