#include "common.h" 
#include "cleanup.h"
#include "newWindow.h"
#include "memoryControl.h"

static int hadCleanUp = 0;

void releaseSharedMemory( char *addr, int shmid, char *comment ) {

    if ( shmdt(addr) < 0)
        err_exit("shmdt error");

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        err_exit("shmctl error");
    else
        pbgreen("remove shared memory identifier successful (%s)", comment);
}

void quit ( Arg *arg ) {

    ShmData *sd = arg->sd;
    MemoryData *med = arg->med;

    if ( hadCleanUp ) return;

    hadCleanUp = 1;

    pbgreen ( "启动清理程序: %d", getpid() );

    if ( med->text != NULL )
        free(med->text);

    if ( med->previousText != NULL )
        free(med->previousText);

    /* FIXME:有时候共享内存会清理不成功*/
    releaseSharedMemory(sd->shmaddr_google, sd->shmid_google, "google");
    releaseSharedMemory(sd->shmaddr_bing, sd->shmid_bing, "baidu");
    releaseSharedMemory(sd->shmaddr_selection, sd->shmid_selection, "selection");
    releaseSharedMemory(sd->shmaddr_searchWin, sd->shmid_searchWin, "searchWin");
    releaseSharedMemory(sd->shmaddr_keyboard, sd->shmid_keyboard, "keyboard");
    releaseSharedMemory(sd->shmaddr_mysql, sd->shmid_mysql, "mysql");
    releaseSharedMemory(sd->shmaddr_pic, sd->shmid_pic, "pic");
    releaseSharedMemory(sd->shmaddr_setting, sd->shmid_setting, "setting");

    releaseMemoryGoogle ( med->google_result );
    releaseMemoryMysql ( med->mysql_result );
    releaseMemoryBing ( med->bing_result );
    releaseMemoryTmp ( med->tmp );

    releaseLink ();
}
