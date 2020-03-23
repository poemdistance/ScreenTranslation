#include "common.h"
#include "cleanup.h"
#include "quickSearch.h"

pid_t quickSearchProcess_pid = 0;
pid_t tranSelect_pid = 0;

extern pthread_t t1;
extern pthread_t t2;
extern pthread_t t3;

static void readChild() {
    waitpid ( quickSearchProcess_pid, NULL, WNOHANG);
}

int main(int argc, char **argv)
{

    setpgid ( getpid(), getpid() );

    pid_t pid;

    if ( (pid = fork()) < 0)
        err_exit("fork err in main");

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
