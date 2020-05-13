#include <stdio.h>
#include <unistd.h>
#include "setting.h"
#include "sharedMemory.h"

    int
main(int argc, char **argv)
{

    if ( fork() == 0 )
    {
        usleep(300*1e3);
        char *shm = NULL;
        shared_memory_for_setting ( &shm );
        shm[0] = '1';
    } 
    else 
    {
        setting();
    }

    return 0;
}

