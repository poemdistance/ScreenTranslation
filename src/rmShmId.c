#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define GETEKYDIR ("/tmp")
#define PROJECTID  (2333)
#define SHMSIZE (1024*1024)

void err_exit(char *arg) {
    fprintf(stderr, "%s\n", arg);
    perror("errno");
    exit(1);
}

    int
main(int argc, char **argv)
{

    key_t key = ftok(GETEKYDIR, PROJECTID);
    if ( key < 0 )
        err_exit("ftok error");

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
    }

    char *addr;

    /* Do not to specific the address to attach
     * and attach for read & write*/
    if ( (addr = shmat(shmid, 0, 0) ) == (void*)-1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
            err_exit("shmctl error");
        else {
            printf("Attach shared memory failed\n");
            printf("remove shared memory identifier successful\n");
        }

        err_exit("shmat error");
    }

    strcpy( addr, "Shared memory test\n" );

    printf("Enter to exit");
    getchar();

    if ( shmdt(addr) < 0) 
        err_exit("shmdt error");

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        err_exit("shmctl error");
    else {
        printf("Finally\n");
        printf("remove shared memory identifier successful\n");
    }

    return 0;
}
