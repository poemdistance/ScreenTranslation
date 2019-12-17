#include "common.h"

int countCharNums (char *source) {

    char *p = source;
    
    int asciinum = 0;
    int nums = 0;

    pbgreen("source phonetci:%s<", source);

    while ( *p ) {

        if ( *p == '|' )
            break;

        if ( (((*p & 0xff) >> 6) & 0x03) == 2  
                && (((*(p+1) & 0xff) >> 6) & 0x03) != 2 )

            nums++;

        if ( (*p&0xff) < 128 && (*p&&0xff >= 0) )
            asciinum++;

        if ( asciinum == 2 ) {
            nums++;
            asciinum = 0;
        }

        p++;
    }

    return nums;
}
