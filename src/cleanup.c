#include "common.h" 
#include "cleanup.h"
#include "newWindow.h"
#include "memoryControl.h"

extern char *shmaddr_selection;
extern char *shmaddr_searchWin;

extern int shmid_google;
extern int shmid_baidu;
extern int shmid_selection;
extern int shmid_searchWin;
extern int shmid_keyboard;
extern int shmid_mysql;
extern int shmid_pic;
extern int shmid_setting;

extern int mousefd;

extern char *text;

extern pid_t baidu_translate_pid;
extern pid_t tranSelect_pid;
extern pid_t google_translate_pid;
extern pid_t check_selectionEvent_pid;
extern pid_t quickSearchProcess_pid;
extern pid_t fetch_data_from_mysql_pid;
extern pid_t detect_tran_pic_action_pid;

extern int BAIDU_TRANS_EXIT_FALG;
extern int GOOGLE_TRANS_EXIT_FLAG;

static int hadCleanUp = 0;

/* TODO:可能会被多次执行，如在终端输入ctrl-c，会被主函数注册的监听SIGINT捕捉到，
 * 之后python翻译程序退出，又接收到SIGCHLD，再被调用一次，最后StranMonitor程序
 * 发送SIGTERM，接收到后又调用一次, 不过影响不大*/

void releaseSharedMemory( char *addr, int shmid, char *comment ) {

    if ( shmdt(addr) < 0)
        err_exit("shmdt error");

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        err_exit("shmctl error");
    else
        pbgreen("remove shared memory identifier successful (%s)", comment);
}

void quit() {

    if ( hadCleanUp )
        return;

    hadCleanUp = 1;

    pbgreen ( "启动清理程序: %d", getpid() );

    if ( text != NULL )
        free(text);

    close(mousefd);

    /* TODO:有时候共享内存会清理不成功*/
    releaseSharedMemory(shmaddr_google, shmid_google, "google");
    releaseSharedMemory(shmaddr_baidu, shmid_baidu, "baidu");
    releaseSharedMemory(shmaddr_selection, shmid_selection, "selection");
    releaseSharedMemory(shmaddr_searchWin, shmid_searchWin, "searchWin");
    releaseSharedMemory(shmaddr_keyboard, shmid_keyboard, "keyboard");
    releaseSharedMemory(shmaddr_mysql, shmid_mysql, "mysql");
    releaseSharedMemory(shmaddr_pic, shmid_pic, "pic");
    releaseSharedMemory(shmaddr_setting, shmid_setting, "setting");


    releaseMemoryGoogle();
    releaseMemoryMysql();
    releaseMemoryTmp();
    releaseLink();

}
