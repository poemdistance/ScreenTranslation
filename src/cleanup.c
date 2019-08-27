#include "common.h" 

extern char *shmaddr_google;
extern char *shmaddr_baidu;

extern int shmid_google;
extern int shmid_baidu;

extern char *baidu_result[BAIDUSIZE];
extern char *google_result[GOOGLESIZE];

extern int mousefd;

extern char *lastText;
extern char *text;
extern int fd_key;

extern pid_t baidu_translate_pid;
extern pid_t google_translate_pid;

extern int BAIDU_TRANS_EXIT_FALG;
extern int GOOGLE_TRANS_EXIT_FLAG;

/* TODO:可能会被多次执行，如在终端输入ctrl-c，会被主函数注册的监听SIGINT捕捉到，
 * 之后python翻译程序退出，又接收到SIGCHLD，再被调用一次，最后StranMonitor程序
 * 发送SIGTERM，接收到后又调用一次, 不过影响不大*/

void quit() {

    printf("\033[0;35m\nCleaning up... \033[0m\n\n");

    if ( text != NULL )
        free(text);

    if ( lastText != NULL )
        free(lastText);

    close(mousefd);
    close(fd_key);

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



    if ( baidu_result[0] != NULL)
        for (int i=0; i<BAIDUSIZE; i++)
            free(baidu_result[i]);

    if ( google_result[0] != NULL)
        for (int i=0; i<GOOGLESIZE; i++)
            free(google_result[i]);


    if ( BAIDU_TRANS_EXIT_FALG != 1 )
        kill ( baidu_translate_pid, SIGKILL );
    if ( GOOGLE_TRANS_EXIT_FLAG != 1 )
        kill ( google_translate_pid, SIGKILL );

    printf("\n");
    exit(0);
}
