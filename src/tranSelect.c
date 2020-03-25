#include "common.h"
#include "sharedMemory.h"
#include "quickSearch.h"
#include "windowData.h"
#include "newWindow.h"
#include "detectMouse.h"
#include "configControl.h"
#include "expanduser.h"

char *shmaddr_google;
char *shmaddr_baidu;
char *shmaddr_selection;
char *shmaddr_searchWin;
char *shmaddr_keyboard;
char *shmaddr_mysql;
char *shmaddr_pic;
char *shmaddr_setting;

int action = 0;
int timeout_id_1;
int timeout_id_2;
int CanNewWin = 0;

int shmid_google;
int shmid_baidu;
int shmid_selection;
int shmid_searchWin;
int shmid_keyboard;
int shmid_mysql;
int shmid_pic;
int shmid_setting;

pthread_t t1 = 0;
pthread_t t2 = 0;
pthread_t t3 = 0;
pthread_t t4 = 0;

extern volatile sig_atomic_t SIGTERM_NOTIFY;

extern volatile sig_atomic_t InNewWin;


void sigterm_notify_cb() {

    SIGTERM_NOTIFY = 1;
}

void *updateConfigData ( void* data ) {

    ConfigData *cd = data;
    struct stat st;
    time_t lastModify = 0;
    char *file = 
        expanduser("/home/$USER/.stran/.configrc");

    while ( 1 ) {

        if ( SIGTERM_NOTIFY ) break;

        stat ( file, &st );

        if ( st.st_mtime - lastModify > 0 ) {
            lastModify = st.st_mtime;
            readNeededValueFromConfig ( cd );
        }

        usleep(1000000);
    }

    return NULL;
}

void *newNormalWindowThread( void *data ) {

    pbblue ( "启动线程 newNormalWindowThread" );

    ConfigData *cd = data;
    /* struct sigaction sa; */
    /* sa.sa_handler = sigterm_notify_cb; */
    /* sigemptyset ( &sa.sa_mask ); */
    /* int ret = 0; */
    /* if ( (ret = sigaction ( SIGTERM, &sa, NULL )) == -1) { */
    /*     pbred("sigaction exec failed (Main.c -> SIGTERM)"); */
    /*     perror("sigaction"); */
    /*     exit(1); */
    /* } */
    /* if ( (ret = sigaction ( SIGINT, &sa, NULL )) == -1) { */
    /*     pbred("sigaction exec failed (Main.c -> SIGTERM)"); */
    /*     perror("sigaction"); */
    /*     exit(1); */
    /* } */

    while ( 1 ) {

        if ( CanNewWin ) {

            pthread_create(&t3, NULL, newNormalWindow, cd);
            pthread_join(t3, NULL);
            CanNewWin = 0;
        }

        if ( SIGTERM_NOTIFY ) break;
        usleep(100000);
    }

    pbcyan ( "newNormalWindowThread 退出" );

    return NULL;
}

void tranSelect() {

    struct Arg arg;
    ConfigData cd;
    arg.cd = &cd;

    char *addr_google;
    char *addr_baidu;
    char *addr_selection;

    pbblue ( "tranSelect 运行:%d", getpid() );

    shmid_google = shared_memory_for_google_translate(&addr_google);
    shmid_baidu = shared_memory_for_baidu_translate(&addr_baidu);
    shmid_selection = shared_memory_for_selection(&addr_selection);
    shmid_searchWin = shared_memory_for_quickSearch( &shmaddr_searchWin );
    shmid_keyboard = shared_memory_for_keyboard_event ( &shmaddr_keyboard );
    shmid_mysql = shared_memory_for_mysql ( &shmaddr_mysql );
    shmid_pic = shared_memory_for_pic ( &shmaddr_pic );
    shmid_setting = shared_memory_for_setting ( &shmaddr_setting );

    shmaddr_google = addr_google;
    shmaddr_baidu = addr_baidu;
    shmaddr_selection = addr_selection;

    memset(shmaddr_google, '0', 10);
    memset(shmaddr_baidu, '0', 10);

    memset(&shmaddr_google[10], '\0', SHMSIZE-10);
    memset(&shmaddr_baidu[10], '\0', SHMSIZE-10);

    arg.addr_google = shmaddr_google;
    arg.addr_baidu = shmaddr_baidu;

    pbblue ( ">>>启动4个线程>>" );

    /*启动鼠标动作检测线程*/
    pthread_create(&t2, NULL, DetectMouse, (void*)&arg);
    pthread_create( &t3, NULL, newNormalWindowThread, (void*)&cd );
    pthread_create( &t4, NULL, updateConfigData, (void*)&cd );

    void *thread_ret;

    while (1) {

        /*启动翻译入口图标线程*/
        pthread_create(&t1, NULL, GuiEntrance, (void*)&arg);
        pthread_join(t1, &thread_ret);
        if ( SIGTERM_NOTIFY ) break;
    }

    pthread_join(t3, &thread_ret); 
    pthread_join(t2, &thread_ret); 
    pthread_join(t4, &thread_ret); 

    pbcyan ( "GuiEntrance 退出" );
}
