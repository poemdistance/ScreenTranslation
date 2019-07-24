/* 注意：
 * 程序退出暂时没有销毁共享内存
 * 等项目后期再来规划相关问题*/

#include "common.h"

char *shmaddr;
int action = 0;
int hideIcon = 0;
int HadDestroied = 0;
int InNewWinFunc = 0;
int timeout_id_1;
int timeout_id_2;

char *text = NULL;
FILE *fp = NULL;
int mousefd;
int fd_key = -1;
int CanNewWin = 0;

int main(int argc, char **argv)
{
    printf("pid=%d\n",getpid ());
    pthread_t t1, t2, t3;
    struct Arg arg;

    char *addr;
    int shmid;
    shmid = shmCreate(&addr);
    shmaddr = addr;

    arg.addr = shmaddr;
    arg.argc = argc;
    arg.argv = argv;

    /*启动鼠标动作检测线程*/
    pthread_create(&t2, NULL, DetectMouse, (void*)&arg);

    void *thread_ret;

    while (1) {

        /*启动翻译入口图标线程*/
        pthread_create(&t1, NULL, GuiEntry, (void*)&arg);
        pthread_join(t1, &thread_ret);
        printf("GuiEntry thread exit captured in All.c\n");

        pthread_create(&t3, NULL, newWindow, NULL);
        pthread_join(t3, &thread_ret);
        printf("New window thread exit captured in All.c\n");

        //   /*进入newWindown函数时不再创建入口图标线程*/
        //   while(InNewWinFunc == 1)
        //       usleep(100000);
    }

    /*TODO:
     * The following codes will never be executed,
     * remember to handle it*/
    pthread_join(t2, &thread_ret); 

    if ( shmdt(shmaddr) < 0)
        err_exit("shmdt error");

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        err_exit("shmctl error");
    else {
        printf("Finally\n");
        printf("remove shared memory identifier successful\n");
    }
}
