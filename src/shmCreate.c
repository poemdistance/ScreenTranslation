#include "common.h"
#include "quickSearch.h"
#include "cleanup.h"

int shared_memory_for_google_translate(char **addr) {

    key_t key = ftok(GETEKYDIR, PROJECTID);
    if ( key < 0 )
        err_exit("ftok error");

    int shmid;
    shmid = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if ( shmid == -1 ) {
        if ( errno == EEXIST ) {
            printf("\033[0;31mshared memeory already exist <google>, \033[0m");
            shmid = shmget(key ,0, 0);
            printf("\033[0;35mreference shmid = %d\033[0m\n", shmid);
        } else {
            perror("errno");
            err_exit("shmget error");
        }
    } else {
        printf("\033[0;35m\nObtained shared memory successful shmid=%d <google>\033[0m\n", shmid);
    }

    /* Do not to specific the address to attach
     * and attach for read & write*/
    if ( (*addr = shmat(shmid, 0, 0) ) == (void*)-1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
            err_exit("shmctl error");
        else {
            printf("Attach shared memory failed <google>\n");
            printf("remove shared memory identifier successful <google>\n");
        }

        err_exit("shmat error <google>");
    } else {
        printf("\033[0;35mAttach to %p <google> \033[0m\n", addr);
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
            printf("\033[0;31mshared memeory already exist <baidu>, \033[0m");
            shmid = shmget(key ,0, 0);
            printf("\033[0;35mreference shmid = %d\033[0m\n", shmid);
        } else {
            perror("errno");
            err_exit("shmget error <baidu>");
        }
    } else {
        printf("\033[0;35mObtained shared memory successful shmid=%d <baidu> \033[0m\n", shmid);
    }

    /* Do not to specific the address to attach
     * and attach for read & write*/
    if ( (*addr = shmat(shmid, 0, 0) ) == (void*)-1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
            err_exit("shmctl error <baidu>");
        else {
            printf("Attach shared memory failed <baidu>\n");
            printf("remove shared memory identifier successful <baidu>\n");
        }

        err_exit("shmat error <baidu>");
    } else {
        printf("\033[0;35mAttach to %p <baidu>\033[0m \n", addr);
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
            printf("\033[0;31mshared memeory already exist <selection>, \033[0m");
            shmid = shmget(key ,0, 0);
            printf("\033[0;35mreference shmid = %d <selection>\033[0m\n", shmid);
        } else {
            perror("errno");
            err_exit("shmget error");
        }
    } else {
        printf("\033[0;35mObtained shared memory successful shmid=%d <selection> \033[0m\n", shmid);
    }

    /* Do not to specific the address to attach
     * and attach for read & write*/
    if ( (*addr = shmat(shmid, 0, 0) ) == (void*)-1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
            err_exit("shmctl error");
        else {
            printf("Attach shared memory failed <selection>\n");
            printf("remove shared memory identifier successful <selection>\n");
        }

        err_exit("shmat error <selection>");
    } else {
        printf("\033[0;35mAttach to %p <selection> \033[0m\n", addr);
    }

    printf("\n");

    return shmid;
}

int shared_memory_for_quickSearch(char **addr) {

    key_t key = ftok(GETEKYDIR, PROJECTID2+2);
    if ( key < 0 )
        err_exit("ftok error");

    int shmid;
    shmid = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if ( shmid == -1 ) {
        if ( errno == EEXIST ) {
            printf("\033[0;31mshared memeory already exist, <quickSearch> \033[0m");
            shmid = shmget(key ,0, 0);
            printf("\033[0;35mreference shmid = %d <quickSearch>\033[0m\n", shmid);
        } else {
            perror("errno");
            err_exit("shmget error <quickSearch>");
        }
    } else {
        printf("\033[0;35mObtained shared memory successful shmid=%d <quickSearch> \033[0m\n", shmid);
    }

    /* Do not to specific the address to attach
     * and attach for read & write*/
    if ( (*addr = shmat(shmid, 0, 0) ) == (void*)-1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
            err_exit("shmctl error");
        else {
            printf("Attach shared memory failed <quickSearch>\n");
            printf("remove shared memory identifier successful <quickSearch>\n");
        }

        err_exit("shmat error <quickSearch>");
    } else {
        printf("\033[0;35mAttach to %p <quickSearch> \033[0m\n", addr);
    }

    printf("\n");

    return shmid;
}

int shared_memory_for_keyboard_event(char **addr) {

    key_t key = ftok(GETEKYDIR, PROJECTID2+3);
    if ( key < 0 )
        err_exit("ftok error");

    int shmid;
    shmid = shmget(key, 100, IPC_CREAT | IPC_EXCL | 0664);
    if ( shmid == -1 ) {
        if ( errno == EEXIST ) {
            printf("\033[0;31mshared memeory already exist, <keyboardEvent> \033[0m");
            shmid = shmget(key ,0, 0);
            printf("\033[0;35mreference shmid = %d <keyboardEvent>\033[0m\n", shmid);
        } else {
            perror("errno");
            err_exit("shmget error <keyboardEvent>");
        }
    } else {
        printf("\033[0;35mObtained shared memory successful shmid=%d <keyboardEvent> \033[0m\n", shmid);
    }

    /* Do not to specific the address to attach
     * and attach for read & write*/
    if ( (*addr = shmat(shmid, 0, 0) ) == (void*)-1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
            err_exit("shmctl error");
        else {
            printf("Attach shared memory failed <keyboardEvent>\n");
            printf("remove shared memory identifier successful <keyboardEvent>\n");
        }

        err_exit("shmat error <keyboardEvent>");
    } else {
        printf("\033[0;35mAttach to %p <keyboardEvent> \033[0m\n", addr);
    }

    printf("\n");

    return shmid;
}

int shared_memory_for_mysql(char **addr) {

    key_t key = ftok(GETEKYDIR, PROJECTID2+4);
    if ( key < 0 )
        err_exit("ftok error");

    int shmid;
    shmid = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if ( shmid == -1 ) {
        if ( errno == EEXIST ) {
            printf("\033[0;31mshared memeory already exist, <mysql> \033[0m");
            shmid = shmget(key ,0, 0);
            printf("\033[0;35mreference shmid = %d <mysql>\033[0m\n", shmid);
        } else {
            perror("errno");
            err_exit("shmget error <mysql>");
        }
    } else {
        printf("\033[0;35mObtained shared memory successful shmid=%d  <mysql>\033[0m\n", shmid);
    }

    /* Do not to specific the address to attach
     * and attach for read & write*/
    if ( (*addr = shmat(shmid, 0, 0) ) == (void*)-1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
            err_exit("shmctl error");
        else {
            printf("Attach shared memory failed\n");
            printf("remove shared memory identifier successful <mysql>\n");
        }

        err_exit("shmat error <mysql>");
    } else {
        printf("\033[0;35mAttach to %p <mysql> \033[0m\n", addr);
    }

    printf("\n");

    return shmid;
}

int shared_memory_for_pic(char **addr) {

    key_t key = ftok(GETEKYDIR, PIC_PROJECT);
    if ( key < 0 )
        err_exit("ftok error");

    int shmid;
    shmid = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if ( shmid == -1 ) {
        if ( errno == EEXIST ) {
            printf("\033[0;31mshared memeory already exist, <pic> \033[0m");
            shmid = shmget(key ,0, 0);
            printf("\033[0;35mreference shmid = %d <pic>\033[0m\n", shmid);
        } else {
            perror("errno");
            err_exit("shmget error <pic>");
        }
    } else {
        printf("\033[0;35mObtained shared memory successful shmid=%d  <pic>\033[0m\n", shmid);
    }

    /* Do not to specific the address to attach
     * and attach for read & write*/
    if ( (*addr = shmat(shmid, 0, 0) ) == (void*)-1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
            err_exit("shmctl error");
        else {
            printf("Attach shared memory failed\n");
            printf("remove shared memory identifier successful <pic>\n");
        }

        err_exit("shmat error <pic>");
    } else {
        printf("\033[0;35mAttach to %p <pic> \033[0m\n", addr);
    }

    printf("\n");

    return shmid;
}
