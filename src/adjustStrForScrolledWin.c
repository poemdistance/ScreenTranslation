#include "common.h"
#include "newWindow.h"
#include "cleanup.h"
#include <ctype.h>

int adjustStrForScrolledWin(int len, char *source) {

    int nowlen = 0;
    int asciich = 0;
    int lines = 1;

    int cansplit = 0;
    char buf[1024] = { '\0' };

    long int srclen = 0;
    srclen = strlen(source);

    if ( srclen > 1024 * 1024 ) {

        fprintf(stderr, "\033[0;31m\n源数据过长，临时数组无法容纳，"\
                "待处理此类情况..., 准备退出...\033[0m\n\n");
        quit();
    }

    /*Be careful here which might be cause stack overflow*/
    char storage[1024*1024] = { '\0' };

    for ( int j=0, k=0; True ; j++, k++ ) 
    {

        /*不复制回车符, --k抵消for循环自增的k*/
        if ( source[j] == '\n' && --k)
            continue;

        if ( source[j] == ' ')
            cansplit = k;

        storage[k] = source[j];

        if ( source[j] == '\0' )
            break;

        if ( (((source[j] & 0xff) >> 6) & 0x03) == 2  
                && (((source[j+1] & 0xff) >> 6) & 0x03) != 2 ) {

            nowlen++;

            if ( k > cansplit )
                cansplit = k;
        }

        if ( (source[j]&0xff) < 128 && (source[j]&&0xff >= 0) )
            asciich++;

        if ( asciich == 2 ) {
            nowlen++;
            asciich = 0;
        }

        if ( nowlen == len ) {

            char nextchar = source[j+1];

            /* 如果不是数字和字母，直接用回车符切割*/
            if ( ! isalnum ( nextchar ) ) {

                storage[++k] = '\n';
                lines++;
                nowlen = 0;
                continue;
            }

            /* 离上一个空格的距离*/
            int count = k - cansplit;
            int right = k+1;
            int left = k;

            if ( count > len || cansplit == 0) {
                storage[++k] = '\n';
                nowlen = 0;
                continue;
            }

            strncpy ( buf, &storage[cansplit+1], count );
            strcat ( buf, "\0" );
            nowlen = countCharNums ( buf );
            k++;

            while ( count-- ) {
                storage[right] = storage[left];
                right--;
                left--;
            }

            storage[left+1] = '\n';
        }
    }

    strcpy(source, storage);

    return lines;
}
