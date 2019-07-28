/* 注意：
 * 程序退出暂时没有销毁共享内存
 * 等项目后期再来规划相关问题*/

#include "common.h"

char *shmaddr;
int action = 0;
int timeout_id_1;
int timeout_id_2;
int CanNewWin = 0;
int shmid;

int main(int argc, char **argv)
{
    printf("pid=%d\n",getpid ());
    pthread_t t1, t2, t3;
    struct Arg arg;

    char *addr;
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

        if ( CanNewWin == 0 )
            continue;

        pthread_create(&t3, NULL, newWindow, NULL);
        pthread_join(t3, &thread_ret);
        printf("New window thread exit captured in All.c\n");

        CanNewWin = 0;
    }

    /*TODO:
     * The following codes will never be executed,
     * remember to handle it*/
    pthread_join(t2, &thread_ret); 

}
