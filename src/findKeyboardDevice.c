#include "quickSearch.h"

char *itoa ( int num ) {

    static char ch[2] = { '\0' };
    sprintf(ch, "%d", num);
    return ch;
}

void err_exit_qs ( const char *buf ) {

    fprintf(stderr, "%s\n", buf);
    perror("error number:");
    fprintf(stderr, "Please try to execute as super user\n");
    exit(1);
}


char *getKeyboardDevice(char *buf)
{
    char basename[100] = "/dev/input/event";
    char devname[100] = { '\0' };
    char info[1024] = { '\0' };

    int fd;

    for ( int i=0; i<20; i++ ) {

        memset ( devname, '\0', sizeof(devname) );
        strcpy ( devname, basename );
        strcat ( devname, itoa(i) );

        if ( access (devname, F_OK) != 0)
            break;

        if ( ( fd = open ( devname, O_RDONLY ) ) < 0 )
            err_exit_qs("open device failed");

        ioctl ( fd, EVIOCGNAME(sizeof(info)), info  );

        close ( fd );

        if ( strstr ( info, "keyboard" ) != NULL ) {

            fprintf(stdout, "\033[0;32mFound keyboard device:%s\033[0;m\n", devname);
            strcpy ( buf, devname );
            return buf;
        }
    }

    return NULL;
}

