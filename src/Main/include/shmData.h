#ifndef __SHM_DATA__
#define __SHM_DATA__

typedef struct {

    char *shmaddr_google;
    char *shmaddr_bing;
    char *shmaddr_selection;
    char *shmaddr_searchWin;
    char *shmaddr_keyboard;
    char *shmaddr_mysql;
    char *shmaddr_pic;
    char *shmaddr_setting;

    int shmid_google;
    int shmid_bing;
    int shmid_selection;
    int shmid_searchWin;
    int shmid_keyboard;
    int shmid_mysql;
    int shmid_pic;
    int shmid_setting;

} ShmData ;

#endif
