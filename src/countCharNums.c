#include "common.h"

int countCharNums (char *source) {

    printf("\033[0;32mIn countCharNums \033[0m\n");

    char *p = source;
    
    int asciinum = 0;
    int nums = 0;

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

        //printf("\033[0;35m (countCharNums)nums=%d \033[0m\n", nums);

        p++;
    }

    printf("\033[0;32mout\033[0m\n");

    return nums;
}
