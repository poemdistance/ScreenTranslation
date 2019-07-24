#include "common.h"

int shmCreate(char **addr) {

    key_t key = ftok(GETEKYDIR, PROJECTID);
    if ( key < 0 )
        err_exit("ftok error");
    printf("key=%d\n", key);

    int shmid;
    shmid = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if ( shmid == -1 ) {
        if ( errno == EEXIST ) {
            printf("shared memeory already exist\n");
            shmid = shmget(key ,0, 0);
            printf("reference shmid = %d\n", shmid);
        } else {
            perror("errno");
            err_exit("shmget error");
        }
    } else {
        printf("shmid=%d\n", shmid);
    }

    /* Do not to specific the address to attach
     * and attach for read & write*/
    if ( (*addr = shmat(shmid, 0, 0) ) == (void*)-1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
            err_exit("shmctl error");
        else {
            printf("Attach shared memory failed\n");
            printf("remove shared memory identifier successful\n");
        }

        err_exit("shmat error");
    } else {
        printf("Attach to %p\n", addr);
    }

    return shmid;
}

