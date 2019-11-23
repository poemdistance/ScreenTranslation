#include "quickSearch.h"

char *itoa ( int num ) {

    static char ch[3] = { '\0' };
    sprintf(ch, "%d", num);
    return ch;
}

void err_exit_qs ( const char *buf ) {

    fprintf(stderr, "%s\n", buf);
    perror("error number:");
    fprintf(stderr, "Please try to execute as super user\n");
    exit(1);
}


char **getKeyboardDevice(char (*buf)[100])
{
    char basename[100] = "/dev/input/event";
    char devname[100] = { '\0' };
    char info[1024] = { '\0' };

    int fd;
    int count = 0;

    for ( int i=0; i<40; i++ ) {

        memset ( devname, '\0', sizeof(devname) );
        strcpy ( devname, basename );
        strcat ( devname, itoa(i) );

        if ( access (devname, F_OK) != 0) {
            //printf("\033[0;31maccess failed %s \033[0m\n\n", devname);
            break;
        }

        if ( ( fd = open ( devname, O_RDONLY ) ) < 0 )
            err_exit_qs("open device failed");

        ioctl ( fd, EVIOCGNAME(sizeof(info)), info  );

        close ( fd );

        //printf("\033[0;35minfo = %s \033[0m\n", info);
        if ( strstr ( info, "Keyboard" ) != NULL || strstr ( info, "keyboard" ) != NULL ) {

            fprintf(stdout, "\033[0;32mFound keyboard device:%s <In findKeyboardDevice.c>\033[0;m\n\n", devname);
            strcpy ( buf[count++], devname );
            continue;
        }
    }

    return (char**)buf;
}

