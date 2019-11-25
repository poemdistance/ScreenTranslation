#include "common.h"
#include "quickSearch.h"
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>

int fd[2];
char buf[2] = { '\0' };
int InSearchWin = 0;
int childExitFlag = 0;

/* Byte 0: quick search 快捷键标志位(alt-j) <for newWindow.c>
 * Byte 1: 退出窗口快捷键标志位(ctrl-c) <for newWindow.c, 目前被屏蔽了>
 * Byte 2: 翻译窗口打开标志位
 * Byte 3: Alt-J 搜索窗口快捷键标志位(好像跟第0字节重复了，太久，啊啦也忘了)
 * Byte 4: 搜索窗口存在标志位
 * */
char *shmaddr_keyboard = NULL;

pid_t searchWindow_pid;
pid_t searchWindowMonitor_pid;
pid_t captureShortcutEvent_pid;

void kill_ourselves() {

    printf("\033[0;31mKILL  captureShortcutEvent(), PID %d \033[0m\n", captureShortcutEvent_pid);
    //kill ( captureShortcutEvent_pid, SIGTERM );

    /* 万一失败就不好了，多来几个*/
    kill ( captureShortcutEvent_pid, SIGKILL );
    kill ( captureShortcutEvent_pid, SIGKILL );
    kill ( captureShortcutEvent_pid, SIGKILL );
    kill ( captureShortcutEvent_pid, SIGKILL );
    kill ( captureShortcutEvent_pid, SIGKILL );
    exit(0);
}

void readChildProcessInfo(int signo) {

    while ( waitpid(searchWindow_pid, NULL, WNOHANG) > 0)
        childExitFlag = 1;
}

void *readSocket() {

    //while (1) {

    //while ( 1 ) {
    //if ( shmaddr_keyboard[3] == '1' ) 
    //break;

    //usleep(2000);
    //}
    //if ( read ( fd[1], buf, 1 ) < 0 )
    //    err_exit_qs ( "read error in quickSearch" );

    //if ( InSearchWin )
    //buf[0] = '0';

    //usleep(1000);
    //}
    //
    return NULL;
}

void quickSearch()
{
    pid_t pid;

    struct sigaction sa;
    sa.sa_handler = kill_ourselves;
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGTERM, &sa, NULL) != 0 )
        err_exit_qs("Sigaction error in quickSearch 1");

    socketpair ( AF_UNIX, SOCK_STREAM, 0, fd );

    if ( (pid = fork()) < 0) 
        err_exit_qs("Fork error");

    /* 父进程*/
    if ( pid > 0 ) {

        shared_memory_for_keyboard_event(&shmaddr_keyboard);

        captureShortcutEvent_pid = pid;
        searchWindowMonitor_pid = getpid();

        sa.sa_handler = readChildProcessInfo;
        sigemptyset ( &sa.sa_mask );
        if ( sigaction ( SIGCHLD, &sa, NULL) != 0 )
            err_exit_qs("Sigaction error in quickSearch 2");

        //printf("\033[0;36mquickSearch pid = %d \033[0m\n", getpid());
        //printf("\033[0;36mquickSearch child pid = %d \033[0m\n", pid);
        close ( fd[0] );

        /* 将键盘监听后写回socket的结果一一读取，防止未读取数据影响程序逻辑*/
        //pthread_t t1;
        //pthread_create ( &t1, NULL, readSocket, NULL );

        while ( 1 ) {

            if ( shmaddr_keyboard[3] == '1') {

                //InSearchWin = 1;
                shmaddr_keyboard[4] = '1';
                shmaddr_keyboard[3] = '0';


                /* 莫得办法，不每次都fork一个进程，窗口除第一次外都无法聚焦*/
                if ( ( pid = fork() ) == 0) {
                    searchWindow();
                    exit(0);
                } 
                else {

                    searchWindow_pid = pid;

                    /* 父进程等待子进程退出*/
                    while ( ! childExitFlag )
                        usleep(10000);

                    childExitFlag = 0;
                    shmaddr_keyboard[4] = '0';
                }

                //InSearchWin = 0;
                //write ( fd[1], "0", 1 );
                //buf[0] = '0';
            }

            usleep(1000);
        }
    } 
    else {

        close ( fd[1] );

        /* 这又是子进程里的，获取到的变量父进程是用不到的!!!!!!!*/
        //captureShortcutEvent_pid = getpid();

        captureShortcutEvent(fd[0]);
    }
}
