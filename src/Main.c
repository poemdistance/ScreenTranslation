#include "common.h"
#include "cleanup.h"
#include "quickSearch.h"

pid_t quickSearchProcess_pid = 0;
pid_t tranSelect_pid = 0;

extern pthread_t t1;
extern pthread_t t2;
extern pthread_t t3;

void sendTerminate() {

    kill( quickSearchProcess_pid, SIGTERM );

    /* 等待子进程退出*/
    //while ( waitpid ( quickSearchProcess_pid, NULL, WNOHANG ) < 0);

    pthread_kill ( t1, SIGTERM );
    pthread_kill ( t2, SIGTERM );
    pthread_kill ( t3, SIGTERM );

    /* 父进程退出*/
    exit(0);
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
    sa.sa_handler = sendTerminate;
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGTERM, &sa, NULL ) == -1) {
        printf("\033[0;31msigaction exec failed (Main.c -> SIGTERM) \033[0m\n");
        perror("sigaction");
        exit(1);
    }

    sa.sa_handler = sendTerminate;
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGINT, &sa, NULL ) == -1) {
        printf("\033[0;31msigaction exec failed (Main.c -> SIGINT) \033[0m\n");
        perror("sigaction");
        exit(1);
    }

    /* 启动取词翻译进程和quickSearch进程*/
    if ( pid > 0 ) {

        quickSearchProcess_pid = pid;
        tranSelect_pid = getpid();
        tranSelectProcess();
    }
    else {

        quickSearchProcess();
    }
}
