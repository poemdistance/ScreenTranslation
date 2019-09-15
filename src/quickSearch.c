#include "common.h"
#include "quickSearch.h"
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>

pid_t capture_shortcut_event;
pid_t searchWindow_pid;

int fd[2];
char buf[2] = { '\0' };
int InSearchWin = 0;
int childExitFlag = 0;

void readChildProcessInfo(int signo) {

    while ( waitpid(searchWindow_pid, NULL, WNOHANG) > 0)
        //while ( waitpid(-1, NULL, WNOHANG) > 0)
        childExitFlag = 1;
}

void *readSocket() {

    while (1) {

        if ( read ( fd[1], buf, 1 ) < 0 )
            err_exit_qs ( "read error in quickSearch" );

        if ( InSearchWin )
            buf[0] = '0';

        usleep(10000);
    }
}

void quickSearch()
{
    pid_t pid;

    socketpair ( AF_UNIX, SOCK_STREAM, 0, fd );

    if ( (pid = fork()) < 0) 
        err_exit_qs("Fork error");

    /* 父进程*/
    if ( pid > 0 ) {

        struct sigaction sa;
        sa.sa_handler = readChildProcessInfo;
        sigemptyset ( &sa.sa_mask );
        if ( sigaction ( SIGCHLD, &sa, NULL) != 0 )
            err_exit_qs("Sigaction error in quickSearch");

        capture_shortcut_event = pid;
        close ( fd[0] );

        /* 将键盘监听后写回socket的结果一一读取，防止未读取数据影响程序逻辑*/
        pthread_t t1;
        pthread_create ( &t1, NULL, readSocket, NULL );

        while ( 1 ) {

            if ( buf[0] == '1') {

                InSearchWin = 1;


                /* 莫得办法，不每次都fork一个进程，窗口除第一次外都无法聚焦*/
                if ( ( pid = fork() ) == 0) {
                    searchWindow();
                    exit(0);
                } 
                else {

                    searchWindow_pid = pid;

                    /* 等待子进程退出*/
                    while ( ! childExitFlag )
                        usleep(200000);

                    childExitFlag = 0;
                }

                InSearchWin = 0;
                write ( fd[1], "0", 1 );
                buf[0] = '0';
            }

            usleep(200000);
        }
    } 
    else {

        close ( fd[1] );
        captureShortcutEvent(fd[0]);
    }
}
