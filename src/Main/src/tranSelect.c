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
#include "shmData.h"
#include "memoryControl.h"

void *updateConfigData ( void* data ) {

    ConfigData        *cd   = ((Arg*)data)->cd;
    CommunicationData *md   = ((Arg*)data)->md;

    struct  stat st;
    time_t  lastModify  = 0;
    char   *file        = expanduser("/home/$USER/.stran/.configrc");

    while ( 1 ) {

        if ( md->sigtermNotify ) break;

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
    CommunicationData *md = ((Arg*)data)->md;

    while ( 1 ) {

        if ( md->canNewWin ) {

            pthread_create(&t3, NULL, newNormalWindow, data);
            pthread_join(t3, NULL);
            md->canNewWin = 0;
        }

        if ( md->sigtermNotify ) break;
        usleep(100000);
    }

    pbcyan ( "newNormalWindowThread 退出" );

    return NULL;
}

void tranSelect ( Arg *arg ) {

    /* ConfigData *cd = arg->cd; */
    CommunicationData *md = arg->md;
    ShmData *sd = arg->sd;
    MemoryData *med = arg->med;

    /*初始化必应以及离线翻译等结果存储空间*/
    initMemoryBing ( med->bing_result );
    initMemoryMysql ( med->mysql_result );
    initMemoryGoogle ( med->google_result );
    initMemoryTmp ( &med->tmp );

    med->text = calloc(TEXTSIZE, sizeof(char));
    med->previousText = calloc(TEXTSIZE, sizeof(char));

    pbred ( "In tranSelect %p %p", md, &(md->sigtermNotify) );

    pthread_t t1 = 0;
    pthread_t t2 = 0;
    pthread_t t3 = 0;
    pthread_t t4 = 0;
    pthread_t t5 = 0;

    pbblue ( "tranSelect 运行:%d", getpid() );

    sd->shmid_google    = shared_memory_for_google_translate ( &sd->shmaddr_google );
    sd->shmid_bing      = shared_memory_for_bing_translate ( &sd->shmaddr_bing );
    sd->shmid_selection = shared_memory_for_selection ( &sd->shmaddr_selection );
    sd->shmid_searchWin = shared_memory_for_quickSearch( &sd->shmaddr_searchWin );
    sd->shmid_keyboard  = shared_memory_for_keyboard_event ( &sd->shmaddr_keyboard );
    sd->shmid_mysql     = shared_memory_for_mysql ( &sd->shmaddr_mysql );
    sd->shmid_pic       = shared_memory_for_pic ( &sd->shmaddr_pic );
    sd->shmid_setting   = shared_memory_for_setting ( &sd->shmaddr_setting );

    memset(sd->shmaddr_google, '0', 10);
    memset(sd->shmaddr_bing, '0', 10);

    memset(&sd->shmaddr_google[10], '\0', SHMSIZE-10);
    memset(&sd->shmaddr_bing[10], '\0', SHMSIZE-10);

    pbblue ( ">>>启动4个线程>>" );

    /*启动鼠标动作检测线程*/
    pthread_create( &t2, NULL, detectMouse, (void*)arg);
    pthread_create( &t3, NULL, newNormalWindowThread, (void*)arg );
    pthread_create( &t4, NULL, updateConfigData, (void*)arg );
    pthread_create( &t5, NULL, listenShortcut, (void*)arg );

    void *thread_ret;

    while (1) {

        /*启动翻译入口图标线程*/
        pthread_create(&t1, NULL, guiEntrance, (void*)arg);
        pthread_join(t1, &thread_ret);
        if ( md->sigtermNotify ) break;
    }

    pthread_join(t3, &thread_ret); 
    pthread_join(t2, &thread_ret); 
    pthread_join(t4, &thread_ret); 

    pbcyan ( "guiEntrance 退出" );
}
