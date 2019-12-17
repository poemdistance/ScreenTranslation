#include "common.h" 
#include "cleanup.h"

extern char *shmaddr_google;
extern char *shmaddr_baidu;
extern char *shmaddr_selection;
extern char *shmaddr_searchWin;
extern char *shmaddr_keyboard;
extern char *shmaddr_mysql;
extern char *shmaddr_pic;

extern int shmid_google;
extern int shmid_baidu;
extern int shmid_selection;
extern int shmid_searchWin;
extern int shmid_keyboard;
extern int shmid_mysql;
extern int shmid_pic;

extern char *baidu_result[BAIDUSIZE];
extern char *google_result[GOOGLESIZE];
extern char *mysql_result[GOOGLESIZE];

extern int mousefd;

extern char *text;
extern int fd_key;

extern pid_t baidu_translate_pid;
extern pid_t tranSelect_pid;
extern pid_t google_translate_pid;
extern pid_t check_selectionEvent_pid;
extern pid_t quickSearchProcess_pid;
extern pid_t fetch_data_from_mysql_pid;
extern pid_t detect_tran_pic_action_pid;

extern int BAIDU_TRANS_EXIT_FALG;
extern int GOOGLE_TRANS_EXIT_FLAG;

extern Display *display;

int hadCleanUp = 0;

/* TODO:可能会被多次执行，如在终端输入ctrl-c，会被主函数注册的监听SIGINT捕捉到，
 * 之后python翻译程序退出，又接收到SIGCHLD，再被调用一次，最后StranMonitor程序
 * 发送SIGTERM，接收到后又调用一次, 不过影响不大*/

void quit() {

    if ( hadCleanUp )
        return;

    hadCleanUp = 1;

    printf("\033[0;35m\nCleaning up... \033[0m\n\n");

    if ( text != NULL )
        free(text);

    close(mousefd);
    close(fd_key);


    /* TODO:有时候共享内存会清理不成功*/

    /* 清除与谷歌翻译的共享内存*/
    if ( shmdt(shmaddr_google) < 0)
        err_exit("shmdt error");

    if (shmctl(shmid_google, IPC_RMID, NULL) == -1)
        err_exit("shmctl error");
    else
        printf("\033[0;32mremove shared memory identifier successful (google)\033[0m\n");



    /* 清除与百度翻译的共享内存*/
    if ( shmdt(shmaddr_baidu) < 0)
        err_exit("shmdt error");

    if (shmctl(shmid_baidu, IPC_RMID, NULL) == -1)
        err_exit("shmctl error");
    else
        printf("\033[0;32mremove shared memory identifier successful (baidu)\033[0m\n");

    /* 清除共享内存*/
    if ( shmdt(shmaddr_selection) < 0)
        err_exit("shmdt error");

    if (shmctl(shmid_selection, IPC_RMID, NULL) == -1)
        err_exit("shmctl error");
    else
        printf("\033[0;32mremove shared memory identifier successful (selection)\033[0m\n");

    /* 清除共享内存*/
    if ( shmdt(shmaddr_searchWin) < 0)
        err_exit("shmdt error");

    if (shmctl(shmid_searchWin, IPC_RMID, NULL) == -1)
        err_exit("shmctl error");
    else
        printf("\033[0;32mremove shared memory identifier successful (search window)\033[0m\n");

    /* 清除共享内存: keyboard event*/
    if ( shmdt(shmaddr_keyboard) < 0)
        err_exit("shmdt error");

    if (shmctl(shmid_keyboard, IPC_RMID, NULL) == -1)
        err_exit("shmctl error");
    else
        printf("\033[0;32mremove shared memory identifier successful (keyboard event)\033[0m\n");
    
    /* 清除共享内存: for mysql*/
    if ( shmdt(shmaddr_mysql) < 0)
        err_exit("shmdt error <mysql>");

    if (shmctl(shmid_mysql, IPC_RMID, NULL) == -1)
        err_exit("shmctl error <mysql>");
    else
        printf("\033[0;32mremove shared memory identifier successful (mysql)\033[0m\n");


    /* 清除共享内存: for pic*/
    if ( shmdt(shmaddr_pic) < 0)
        err_exit("shmdt error <pic>");

    if (shmctl(shmid_pic, IPC_RMID, NULL) == -1)
        err_exit("shmctl error <pic>");
    else
        printf("\033[0;32mremove shared memory identifier successful (pic)\033[0m\n");

    /* 释放翻译结果存储空间 <Baidu>*/
    if ( baidu_result[0] != NULL)
        for (int i=0; i<BAIDUSIZE; i++)
            free(baidu_result[i]);

    /* 释放翻译结果存储空间 <Google>*/
    if ( google_result[0] != NULL)
        for (int i=0; i<GOOGLESIZE; i++)
            free(google_result[i]);

    /* 释放翻译结果存储空间 <Mysql>*/
    if ( mysql_result[0] != NULL)
        for (int i=0; i<MYSQLSIZE; i++)
            free(mysql_result[i]);


    if ( BAIDU_TRANS_EXIT_FALG != 1 )
        kill ( baidu_translate_pid, SIGKILL );
    if ( GOOGLE_TRANS_EXIT_FLAG != 1 )
        kill ( google_translate_pid, SIGKILL );

    kill ( check_selectionEvent_pid, SIGKILL );
    kill ( quickSearchProcess_pid, SIGTERM );
    kill ( fetch_data_from_mysql_pid, SIGTERM );
    kill ( detect_tran_pic_action_pid, SIGTERM );

    /* 进程所在文件也进行了清理，判断一下防止多次释放,
     * (释放后display会被置为空)*/
    if ( display )
        XCloseDisplay(display);

    /* 手动再赋值空，保险一点*/
    display = NULL;

    printf("\n");
    exit(0);
}
