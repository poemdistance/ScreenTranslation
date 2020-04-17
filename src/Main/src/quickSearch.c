#include "common.h"
#include "quickSearch.h"
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include "sharedMemory.h"
#include "cleanup.h"

static char *shmaddr_keyboard = NULL;
static pid_t captureShortcutEvent_pid;
static int SIGTERM_SIGNAL = 0;


static void readChild() {

    while( waitpid(captureShortcutEvent_pid, NULL, WNOHANG) > 0);
}

void kill_ourselves() {

    SIGTERM_SIGNAL = 1;
}

void quickSearch()
{
    pbyellow ( "启动 quick search" );

    pid_t pid;

    struct sigaction sa;
    sigemptyset ( &sa.sa_mask );
    sa.sa_handler = kill_ourselves;
    if ( sigaction ( SIGTERM, &sa, NULL) != 0 )
        err_exit("Sigaction error in quickSearch 1");
    if ( sigaction ( SIGINT, &sa, NULL) != 0 )
        err_exit("Sigaction error in quickSearch 1");
    sa.sa_handler = readChild;
    if ( sigaction ( SIGCHLD, &sa, NULL) != 0 )
        err_exit("Sigaction error in quickSearch 1");


    shared_memory_for_keyboard_event(&shmaddr_keyboard);
    memset(shmaddr_keyboard, '0', 100);

    if ( (pid = fork()) < 0) 
        err_exit("Fork error");

    /* 父进程*/
    if ( pid > 0 ) {

        setproctitle ( "%s", "Quick Search" );

        captureShortcutEvent_pid = pid;

        while ( 1 ) {

            if ( shmaddr_keyboard[QUICK_SEARCH_FLAG] == '1') {

                pmag ( "启动quick search 窗口" );

                shmaddr_keyboard[SEARCH_WINDOW_OPENED_FLAG] = '1';
                shmaddr_keyboard[QUICK_SEARCH_FLAG] = '0';


                /* 莫得办法，不每次都fork一个进程，窗口除第一次外都无法聚焦*/
                if ( ( pid = fork() ) == 0) {
                    searchWindow();
                    pbblue ( "searchWindow() 退出" );
                    shmaddr_keyboard[QUICK_SEARCH_FLAG] = '0';
                    exit(0);
                } 
                else {

                    pbblue("等待搜索窗口退出");

                    /* wait(pid)*/
                    waitpid(pid, NULL, 0);

                    shmaddr_keyboard[SEARCH_WINDOW_OPENED_FLAG] = '0';
                    pbblue("搜索窗口已退出");
                }
            }

            usleep(10000);

            if ( SIGTERM_SIGNAL ) break;
        }
    } 

    pbcyan ( "Quick Search 程序退出: %d", getpid() );
}
