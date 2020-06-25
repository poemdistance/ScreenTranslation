#include "common.h"
#include "cleanup.h"
#include "quickSearch.h"
#include "windowData.h"
#include "shmData.h"

static volatile sig_atomic_t *sigterm_variable_pointer = NULL;

static Arg *arg_global = NULL;
static ShmIdData *sid_global = NULL;

static void readChild() {

#if 0

    pid_t pid;
    while ( ( pid = waitpid ( -getpid(), NULL, WNOHANG ) ) > 0 ) {
        if ( pid == sid_global->pid_mysql ) 
        {
            pbmag ( "fetchDict exit" );
        }
        else if (
                pid == sid_global->pid_selection ||
                pid == sid_global->pid_bing      ||
                pid == sid_global->pid_google    ||
                pid == sid_global->pid_tranpic   ) 
        {

            pbred ( "---------- Our child process exit !!!! --------------" );
            *sigterm_variable_pointer = 1;
        }
        else
        {
            pbred ( "Unknow child precess exit" );
        }
    }
#endif

#if 1
    while ( waitpid ( sid_global->pid_selection, NULL, WNOHANG ) > 0 );
    while ( waitpid ( sid_global->pid_bing, NULL, WNOHANG ) > 0 );
    while ( waitpid ( sid_global->pid_google, NULL, WNOHANG ) > 0 );
    while ( waitpid ( sid_global->pid_tranpic, NULL, WNOHANG ) > 0 );
    while ( waitpid ( sid_global->pid_mysql, NULL, WNOHANG ) > 0 );
#endif
}

static void sigterm() {

    *sigterm_variable_pointer = 1;
    quit ( arg_global );
}

int main(int argc, char **argv)
{
    Arg arg;
    ConfigData cd;
    CommunicationData md;
    ShmData sd;
    ShmIdData sid;
    MemoryData med;

    memset ( (void*)&cd, 0, sizeof(cd) );
    memset ( (void*)&md, 0, sizeof(md) );
    memset ( (void*)&sd, 0, sizeof(sd) );
    memset ( (void*)&sid, 0, sizeof(sid) );
    memset ( (void*)&med, '\0', sizeof(med) );

    arg.cd = &cd;
    arg.md = &md;
    arg.sd = &sd;
    arg.sid = &sid;
    arg.med = &med;

    sigterm_variable_pointer = &(md.sigtermNotify);
    arg_global = &arg;
    sid_global = &sid;

    setpgid ( getpid(), getpid() );

    pid_t pid;

    if ( (pid = fork()) < 0) {
        /* err_exit("fork err in main"); */
        pbred ( "Fork error in main" );
        exit(1);
    }

    void (*quickSearchProcess)(void);
    quickSearchProcess = quickSearch;

    void (*tranSelectProcess)( Arg* );
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
        tranSelectProcess ( &arg );
    }
    else {

        quickSearchProcess();
    }

    sleep(2);
}
