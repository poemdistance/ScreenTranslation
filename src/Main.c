/* 注意：
 * 程序退出暂时没有销毁共享内存
 * 等项目后期再来规划相关问题*/

#include "common.h"

char *shmaddr_google;
char *shmaddr_baidu;

int action = 0;
int timeout_id_1;
int timeout_id_2;
int CanNewWin = 0;

int shmid_google;
int shmid_baidu;

int main(int argc, char **argv)
{
    pthread_t t1, t2, t3;
    struct Arg arg;

    char *addr_google;
    char *addr_baidu;

    shmid_google = shared_memory_for_google_translate(&addr_google);
    shmid_baidu = shared_memory_for_baidu_translate(&addr_baidu);


    shmaddr_google = addr_google;
    shmaddr_baidu = addr_baidu;

    memset(shmaddr_google, '0', (size_t)10);
    memset(shmaddr_baidu, '0', (size_t)10);

    memset(shmaddr_google, '\0', SHMSIZE-10);
    memset(shmaddr_baidu, '\0', SHMSIZE-10);

    arg.addr_google = shmaddr_google;
    arg.addr_baidu = shmaddr_baidu;
    arg.argc = argc;
    arg.argv = argv;

    /*启动鼠标动作检测线程*/
    pthread_create(&t2, NULL, DetectMouse, (void*)&arg);

    void *thread_ret;


    /* 捕获终止(Terminate)程序信号，并调用退出函数清理相关资源,
     * 如释放内存，终止翻译端程序*/
    struct sigaction sa;
    sa.sa_handler = quit;
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGTERM, &sa, NULL ) == -1) {
        printf("\033[0;31msigaction exec failed (Main.c -> SIGTERM) \033[0m\n");
        perror("sigaction");
        exit(1);
    }

    signal(SIGINT, quit);

    while (1) {

        /*启动翻译入口图标线程*/
        pthread_create(&t1, NULL, GuiEntrance, (void*)&arg);
        pthread_join(t1, &thread_ret);

        if ( CanNewWin == 0 )
            continue;

        pthread_create(&t3, NULL, newNormalWindow, NULL);
        pthread_join(t3, &thread_ret);

        CanNewWin = 0;
    }

    /*TODO:
     * The following codes will never be executed,
     * remember to handle it*/
    pthread_join(t2, &thread_ret); 
}
