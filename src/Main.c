#include "common.h"
#include "cleanup.h"
#include "quickSearch.h"

pid_t quickSearchProcess_pid = 0;
pid_t tranSelect_pid = 0;

extern pthread_t t1;
extern pthread_t t2;
extern pthread_t t3;

void sendTerminate() {

    /* 等待子进程退出*/
    //while ( waitpid ( quickSearchProcess_pid, NULL, WNOHANG ) < 0);

    pbcyan ( "发送SIGTERM信号到3个线程" );

    /* 向线程发送信号*/
    pthread_kill ( t1, SIGTERM );
    pthread_kill ( t2, SIGTERM );
    pthread_kill ( t3, SIGTERM );

    kill( quickSearchProcess_pid, SIGTERM );

    /* 父进程退出*/
    exit(0);
}

static void readChild() {
    waitpid ( quickSearchProcess_pid, NULL, WNOHANG);
}

int main(int argc, char **argv)
{

    pid_t pid;

    if ( (pid = fork()) < 0)
        err_exit("fork err in main");

    void (*quickSearchProcess)(void);
    quickSearchProcess = quickSearch;

    void (*tranSelectProcess)(void);
    tranSelectProcess = tranSelect;

    struct sigaction sa;
    /* sa.sa_handler = sendTerminate; */
    /* sigemptyset ( &sa.sa_mask ); */
    /* if ( sigaction ( SIGTERM, &sa, NULL ) ) { */
    /*     printf("\033[0;31msigaction exec failed (Main.c -> SIGTERM) \033[0m\n"); */
    /*     perror("sigaction"); */
    /*     exit(1); */
    /* } */


    sa.sa_handler = readChild;
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGCHLD, &sa, NULL ) == -1) {
        pbred("sigaction exec failed (Main.c -> SIGCHLD)");
        perror("sigaction");
        exit(1);
    }

    /* 启动取词翻译进程和quickSearch进程*/
    if ( pid > 0 ) {

        quickSearchProcess_pid = pid;
        tranSelect_pid = getpid();
        tranSelectProcess();
        quit();
    }
    else {

        quickSearchProcess();
    }

    /* 处理函数不要放到这里，父子进程都可以执行到这*/

    pbcyan ( "主函数退出" );
}
