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

void quit() {

    printf("clean up...\n");

    /*退出前加个回车*/
    fprintf(stdout, "\n");

    if ( text != NULL )
        free(text);

    if ( lastText != NULL )
        free(lastText);

    close(mousefd);
    close(fd_key);

    if ( shmdt(shmaddr_google) < 0)
        err_exit("shmdt error");

    printf("\n");
    if (shmctl(shmid_google, IPC_RMID, NULL) == -1)
        err_exit("shmctl error");
    else {
        printf("remove shared memory identifier successful (google)\n");
    }

    if ( shmdt(shmaddr_baidu) < 0)
        err_exit("shmdt error");

    if (shmctl(shmid_baidu, IPC_RMID, NULL) == -1)
        err_exit("shmctl error");
    else {
        printf("remove shared memory identifier successful (baidu)\n");
    }

    if ( baidu_result[0] != NULL)
        for (int i=0; i<BAIDUSIZE; i++)
            free(baidu_result[i]);

    if ( google_result[0] != NULL)
        for (int i=0; i<GOOGLESIZE; i++)
            free(google_result[i]);

    printf("\n");

    exit(1);
}
