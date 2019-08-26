#include "common.h"

int adjustStrForScrolledWin(int len, char *source) {

    int nowlen = 0;
    int asciich = 0;
    int lines = 1;

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

        storage[k] = source[j];

        if ( source[j] == '\0' )
            break;

        if ( (((source[j] & 0xff) >> 6) & 0x03) == 2  
                && (((source[j+1] & 0xff) >> 6) & 0x03) != 2 )

            nowlen++;

        if ( (source[j]&0xff) < 128 && (source[j]&&0xff >= 0) )
            asciich++;

        if ( asciich == 2 ) {
            nowlen++;
            asciich = 0;
        }

        if ( nowlen == len ) {
            storage[++k] = '\n';
            nowlen = 0;
            lines++;
        }
    }

    strcpy(source, storage);

    return lines;
}
