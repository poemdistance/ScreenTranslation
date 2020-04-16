#include "common.h"
#include "cleanup.h"
#include "quickSearch.h"

static pid_t quickSearchProcess_pid = 0;

volatile sig_atomic_t SIGTERM_NOTIFY = 0;
extern pid_t pid_mysql;

static void readChild() {

    pid_t pid;
    /* waitpid ( quickSearchProcess_pid, NULL, WNOHANG); */
    while ( ( pid = waitpid ( -1, NULL, WNOHANG ) ) > 0 ) {
        pbred ( "Child status changed. PID:%d", pid );
        if ( pid == pid_mysql ) {
            pbmag ( "Yes!!! mysql exit" );
        }
        else
            SIGTERM_NOTIFY = 1;
    }
}

static void sigterm() {
    SIGTERM_NOTIFY = 1;
    quit();
}

int main(int argc, char **argv)
{

    setpgid ( getpid(), getpid() );

    pid_t pid;

    if ( (pid = fork()) < 0) {
        /* err_exit("fork err in main"); */
        pbred ( "Fork error in main" );
        exit(1);
    }

    void (*quickSearchProcess)(void);
    quickSearchProcess = quickSearch;

    void (*tranSelectProcess)(void);
    tranSelectProcess = tranSelect;

    /* 由于是多线程，程序接收到SIGTERM信号只会发给其中
     * 一个线程，主进程这里取消接收SIGTERM信号，改由
     * transelect进程里的newNormalWindow线程接收，接收后
     * 标记一个退出变量，其他线程轮询此变量，置1时各个线
     * 程退出, 最终在下面的quit()里执行清理资源的事项，
     * 顺便杀掉quick search 进程,至此整个翻译程序退出*/
    struct sigaction sa;
    sa.sa_handler = readChild;
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGCHLD, &sa, NULL ) == -1) {
        pbred("sigaction exec failed (Main.c -> SIGCHLD)");
        perror("sigaction:");
        exit(1);
    }

    /* 启动取词翻译进程和quickSearch进程*/
    if ( pid > 0 ) {

        sa.sa_handler = sigterm;
        if ( sigaction ( SIGTERM, &sa, NULL ) == -1) {
            pbred("sigaction exec failed (Main.c -> SIGTERM)");
            perror("sigaction");
            exit(1);
        }
        sa.sa_handler = sigterm;
        if ( sigaction ( SIGINT, &sa, NULL ) == -1) {
            pbred("sigaction exec failed (Main.c -> SIGINT)");
            perror("sigaction");
            exit(1);
        }
        quickSearchProcess_pid = pid;
        tranSelectProcess();
    }
    else {

        quickSearchProcess();
    }

    sleep(2);
}
