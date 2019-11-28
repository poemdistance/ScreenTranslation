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

int PROCESS_EXIT_FLAG = 0;
int CTRL_C_FLAG = 0;
int SIG_KILL_FLAG = 0;

pid_t needWait = 0;

void handler()  { 

    pid_t pid = 0;

    while ( ( pid = waitpid (needWait, NULL, WNOHANG) ) > 0 ) {
        /* 其实这里可以不用加if，因为如果不是这个程序退出返回0不会进入这里*/
        if ( pid == needWait )
            PROCESS_EXIT_FLAG = 1;
    }
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

void sigkill() {

    SIG_KILL_FLAG = 1;
    kill( needWait, SIGTERM );
    usleep(1000000);
    exit(0);
}

    int
main(int argc, char **argv)
{
    struct sigaction sa;
    pid_t pid = 0;

    printf("\033[0;35mMonitor Process %d \033[0m\n", getpid());

    sa.sa_handler = handler;;
    sigemptyset ( &sa.sa_mask );
    sa.sa_flags = SA_RESTART;
    if ( sigaction ( SIGCHLD, &sa, NULL ) == -1) {
        printf("\033[0;31msigaction failed to exec (SIGCHLD) \033[0m\n");
        exit(1);
    }

    sa.sa_handler = ctrl_c; 
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGINT, &sa, NULL ) == -1) {
        printf("\033[0;31msigaction failed to exec (SIGINT)\033[0m\n");
        exit(1);
    }

    sa.sa_handler = sigkill; 
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGTERM, &sa, NULL ) == -1) {
        printf("\033[0;31msigaction failed to exec (SIGTERM)\033[0m\n");
        exit(1);
    }

    pid = fork();

    /* 父进程：
     *
     * 监控子程序状态，如果子程序退出，则再fork一个子进程重新启动之*/
    if ( pid > 0 ) {
        needWait = pid;
        printf("\033[0;34m ( Monitor ) Parent process, child pid =%d\033[0m\n", pid);
        while ( 1 ) { 
            usleep ( 1000000 );
            if ( PROCESS_EXIT_FLAG == 1 && CTRL_C_FLAG != 1 && SIG_KILL_FLAG != 1) {
                printf("\033[0;31mstran 子进程已退出, 准备重新启动... \033[0m\n");

                PROCESS_EXIT_FLAG = 0;
                pid = fork();

                if ( pid > 0 ) {
                    needWait = pid;
                    printf("\033[0;34m(Monitor) Parent process in while, new child process id %d \033[0m\n", pid);
                }

                /* 子进程需要跳出while循环进入下面的翻译程序启动代码*/
                if ( pid == 0 )
                    break;

                if ( CTRL_C_FLAG == 1 )
                    break;
            }
        }
    }

    if ( pid == 0 ) {

        if (execl ( "/usr/bin/stran", "stran", NULL ) == -1) {
            printf("\033[0;31mexecl error occured \033[0m\n");
            perror("execl errno");
            exit(1);
        }
    }

    return 0;
}

