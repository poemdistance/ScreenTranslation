#include "common.h"

#define or ||

int isEmpty(char *buf) {

    if ( ! buf )
        return 1;

    char *p = buf;
    while ( *p ) {
        if ( *p == ' ' or *p == '\n' )
            p++;
        else
            return 0;
    }

    return 1;
}
