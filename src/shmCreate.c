#include "common.h"

int shared_memory_for_google_translate(char **addr) {

    key_t key = ftok(GETEKYDIR, PROJECTID);
    if ( key < 0 )
        err_exit("ftok error");

    int shmid;
    shmid = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if ( shmid == -1 ) {
        if ( errno == EEXIST ) {
            printf("\033[0;31mshared memeory already exist,\033[0m");
            shmid = shmget(key ,0, 0);
            printf("\033[0;35mreference shmid = %d\033[0m\n", shmid);
        } else {
            perror("errno");
            err_exit("shmget error");
        }
    } else {
        printf("\033[0;35m\nObtained shared memory successful shmid=%d \033[0m\n", shmid);
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
        printf("\033[0;35mAttach to %p \033[0m\n", addr);
    }

    printf("\n");

    return shmid;
}


int shared_memory_for_baidu_translate(char **addr) {

    key_t key = ftok(GETEKYDIR, PROJECTID2);
    if ( key < 0 )
        err_exit("ftok error");

    int shmid;
    shmid = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if ( shmid == -1 ) {
        if ( errno == EEXIST ) {
            printf("\033[0;31mshared memeory already exist,\033[0m");
            shmid = shmget(key ,0, 0);
            printf("\033[0;35mreference shmid = %d\033[0m\n", shmid);
        } else {
            perror("errno");
            err_exit("shmget error");
        }
    } else {
        printf("\033[0;35mObtained shared memory successful shmid=%d \033[0m\n", shmid);
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
        printf("\033[0;35mAttach to %p \033[0m\n", addr);
    }

    printf("\n");

    return shmid;
}

int shared_memory_for_selection(char **addr) {

    key_t key = ftok(GETEKYDIR, PROJECTID2+1);
    if ( key < 0 )
        err_exit("ftok error");

    int shmid;
    shmid = shmget(key, 10, IPC_CREAT | IPC_EXCL | 0664);
    if ( shmid == -1 ) {
        if ( errno == EEXIST ) {
            printf("\033[0;31mshared memeory already exist,\033[0m");
            shmid = shmget(key ,0, 0);
            printf("\033[0;35mreference shmid = %d\033[0m\n", shmid);
        } else {
            perror("errno");
            err_exit("shmget error");
        }
    } else {
        printf("\033[0;35mObtained shared memory successful shmid=%d \033[0m\n", shmid);
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
        printf("\033[0;35mAttach to %p \033[0m\n", addr);
    }

    printf("\n");

    return shmid;
}

