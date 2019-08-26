#include "common.h"

int countLines ( int len, char *source  ) {

    char *p = source;
    int nowlen = 0;
    int asciinum;
    int lines = 1;

    while ( *p ) {

        if ( *p == '|' )
            break;

        if ( (((*p & 0xff) >> 6) & 0x03) == 2  
                && (((*(p+1) & 0xff) >> 6) & 0x03) != 2 )

            nowlen++;

        if ( (*p&0xff) < 128 && (*p&&0xff >= 0) )
            asciinum++;

        if ( asciinum == 2 ) {
            nowlen++;
            asciinum = 0;
        }

        if ( nowlen == len ) {
            nowlen = 0;
            lines++;
        }

        p++;
    }

    return lines;
}

