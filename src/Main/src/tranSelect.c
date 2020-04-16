#include "common.h"
#include "sharedMemory.h"
#include "quickSearch.h"
#include "windowData.h"
#include "newWindow.h"
#include "detectMouse.h"
#include "configControl.h"
#include "expanduser.h"
#include "pointer.h"
#include "strmask.h"

char *shmaddr_google;
char *shmaddr_baidu;
char *shmaddr_selection;
char *shmaddr_searchWin;
char *shmaddr_keyboard;
char *shmaddr_mysql;
char *shmaddr_pic;
char *shmaddr_setting;

int CanNewWin = 0;

int shmid_google;
int shmid_baidu;
int shmid_selection;
int shmid_searchWin;
int shmid_keyboard;
int shmid_mysql;
int shmid_pic;
int shmid_setting;

extern volatile sig_atomic_t SIGTERM_NOTIFY;
extern volatile sig_atomic_t InNewWin;


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

    pthread_t t3;

    while ( 1 ) {

        if ( CanNewWin ) {

            pthread_create(&t3, NULL, newNormalWindow, data);
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

    Arg arg;
    ConfigData cd;
    CommunicationData md;
    arg.cd = &cd;
    arg.md = &md;

    pthread_t t1 = 0;
    pthread_t t2 = 0;
    pthread_t t3 = 0;
    pthread_t t4 = 0;
    pthread_t t5 = 0;

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

    pbblue ( ">>>启动4个线程>>" );

    /*启动鼠标动作检测线程*/
    pthread_create(&t2, NULL, DetectMouse, (void*)&arg);
    pthread_create( &t3, NULL, newNormalWindowThread, (void*)&arg );
    pthread_create( &t4, NULL, updateConfigData, (void*)&cd );
    pthread_create( &t5, NULL, listenShortcut, (void*)&arg );

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
