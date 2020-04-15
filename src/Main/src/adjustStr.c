#include "common.h"
#include "newWindow.h"
#include <ctype.h>

int theThirdByteOfChinese ( char *c ) {

    /*此处需要了解一下utf8编码格式
     *
     * 对于只有一个字节的字符,单字节最高位为0，
     * 否则最高位开始往后数，有多少个1，就代表
     * 当前字符有多少字节，utf-8在本机测试为3个字节，
     * utf-8格式为111xxx 10xxxx 10xxxx(二进制表示).
     * 所以只要判断到当前字节的高两位为10,即十进制为2，且下一个字节
     * 的高两位不是10，即不是十进制中的2，说明单个汉字已经读取结束
     * 这个时候才可以对该字符后进行截取(加回车符),不然可能导致乱码
     *
     * 另说明这里与上0xff纯粹是为了有时打印成int型值时提供方便,因为int型
     * 有4个字节，需要截断才能显示正确的值*/
    return \
        (((*c & 0xff) >> 6) & 0x03) == 2  \
        && (((*(c+1) & 0xff) >> 6) & 0x03) != 2;
}

int isAllAscii ( char *str ) {

    char *p = str;
    while ( p && *p && isascii ( *p ) ) p++;
    return p && *p == '\0';
}

/* 检测字符串是否少于N个计数长度
 *
 * 两个 字母或者数字 作为一个计数长度
 * 一个中文字符作为一个计数长度*/
int isLessNLen( char *str, int num ) {

    if ( !str ) return TRUE;

    if ( isAllAscii ( str ) )
        return (strlen ( str ) / 2) <= num;

    char *p = str;
    char count = 0;
    int len = 0;
    while ( *p ) {

        if ( isascii ( *p ) && *p != '\n' ) ++count;
        if ( count == 2 && ( (count = 0) || 1 ) ) ++len;
        if ( theThirdByteOfChinese ( p ) ) ++len;
        p++;
    }

    return len <= num;
}

/*字符串调整函数:
 * 向每超过一定长度的字符串中添加回车字符,避免单行过长
 *
 * p[]指针数组，指向若干个待处理字符串
 * len 为单行所需截取长度
 * storage[] 指针数组，用来存储处理后的字符串
 */
void adjustStrForGoogle(char *p[], int len, char *storage[], int *enterNum) {

    int nowlen = 0; /* 当前计数长度*/
    int count = 0;
    int cansplit = 0;

    for ( int i=0; i<3; i++ ) 
    {
        nowlen = 0;

        for ( int j=0, k=0; p[i] != NULL ; j++, k++ ) 
        {
            storage[i][k] = p[i][j];

            /*读到结尾字符时退出内层for循环，处理下一个字符串*/
            if ( p[i][j] == '\0' ) {
                /* if ( j != 0 && i != 0) { */
                /*     strcat ( storage[i], "\n" ); */
                /* } */
                break;
            }

            if ( p[i][j] == '\n' ) continue;

            /* 空格字符可以被分割，下标记录于canSplit*/
            if ( p[i][j] == ' ') cansplit = k;

            if (theThirdByteOfChinese(&p[i][j])) {
                nowlen++;
                /* 汉字编码的最后一个字节也可以分割,
                 * 所以这里需要更新最后一个可分割字符的下标到canSplit*/
                if ( k > cansplit ) cansplit = k;
            }

            if ( isascii ( p[i][j] ) ) count++;

            if ( count == 2 ) {
                nowlen++;
                count = 0;
            }

            /* nowlen==len表示应该加回车符了，
             * 而两个isalnum来判断当前位置是不是在单词内部,
             * 如果是，则加回车符的位置应该回退到上一个可分割
             * 字符那. 从而防止单词被割裂到上下行之间*/
            if (nowlen == len && isalnum(p[i][j]) && isalnum(p[i][j+1]))
            {
                /* 待回退字符长度*/
                int count = k - cansplit;

                /* 排除可分割下标在行头或者待回退字符过长的情况,此种情况强制切割*/
                if (! (count > len || k == 0)) {

                    int goback = 0;
                    while (isalnum(p[i][j--]))
                        storage[i][k - goback++] = ' ';
                }
            }

            if ( nowlen == len ) {

                storage[i][++k] = '\n';

                (*enterNum)++;

                /* 如果当前剩余字符的计数长度小于2，则没有必要
                 * 添加回车符, --k就是使下次复制字符的时候将上
                 * 一次的回车符覆盖掉*/
                if ( isLessNLen ( &p[i][j], 3 ) ) --k;

                nowlen = 0;
                continue;
            }
        }
    }
}

void adjustStrForBaidu(int len, char *source, int addSpace, int copy, int *enterNum) {

    int nowlen = 0;
    int count = 0;
    int prefixLen = 0;
    int start = 0;
    int cansplit = 0;
    int notlock = 1;

    char buf[1024] = { '\0' };

    long int srclen = 0;
    srclen = strlen(source);

    if ( srclen > 1024 * 1024 ) {

        pbred("源数据过长，临时数组无法容纳");
        return;
    }

    char storage[1024*1024] = { '\0' };

    for ( int j=0, k=0; True ; j++, k++ ) 
    {
        storage[k] = source[j];

        if ( source[j] == '\0' ) break;
        if ( source[j] == '\n' ) continue;

        /* 空格为可分割字符，记录下该下标*/
        if ( source[j] == ' ') cansplit = k;

        if ( addSpace && source[j] == '.' && notlock ) {

            prefixLen = (j+1-start)*2;
            nowlen = (j-start)/2;
            notlock = 0;
        }

        /* 检测当前字节是否为单个汉字的最后一个字节*/
        if (theThirdByteOfChinese(&source[j])) {

            nowlen++;

            /* 汉字可分割，如果下标大于上一个可分割空格字符的，则替换之*/
            if ( k > cansplit ) cansplit = k;
        }

        if ( isascii(source[j]) ) count++;

        /* 两个ascii码作一个计数长度*/
        if ( count == 2 && ( (count=0 ) || 1 )) nowlen++;

        if ( nowlen == len ) {

            if ( isLessNLen ( &source[j+1], 3 ) && ( ( nowlen = 0 ) || 1 ) ) continue;

            char nextchar = source[j+1];
            int forward = 0;
            int start = 0;
            int end = 0;

            /* 如果下一个字符不是字母或数字，直接用回车符切割*/
            if ( ! isalnum(nextchar) ) {
                storage[++k] = '\n';
                (*enterNum)++;
                nowlen = 0;
            } 
            else {

                /* 离上一个可分割字符的距离*/
                int count = k - cansplit;
                int right = k+1;
                int left = k;

                /* 如果上一个可分割字符在起点或者
                 * 当前位置离上一个可分割字符超过一行的长度,
                 * 那么强制分割，并返回for循环*/
                if ( count > len || cansplit == 0) {
                    storage[++k] = '\n';
                    (*enterNum)++;
                    nowlen = 0;
                    continue;
                }

                /*分割点前移了，再加回车符使原本已复制好的数据后移，先备份*/
                strncpy ( buf, &storage[cansplit+1], count );
                strcat ( buf, "\0" );

                /*后移的数据作为新行，同时其长度为当前新行长度*/
                nowlen = countCharNums ( buf );

                if ( addSpace ) {
                    right = right + prefixLen;
                    end = right; 
                }

                /*分割点后面原先数据整体后移一个字节*/
                while ( count-- ) {
                    storage[right] = storage[left];
                    right--;
                    left--;
                }

                storage[left+1] = '\n';
                (*enterNum)++;

                k++;
                start = left + 2;
                forward = 1;
            }

            if ( addSpace ) {

                if ( forward == 1 ) k = start-1;

                int tmp = prefixLen;
                while(tmp--) {
                    storage[++k] = ' ';
                    nowlen++;
                }

                nowlen -= prefixLen/2;

                if ( forward == 1 ) k = end;
            }
            continue;
        }
    }

    if ( copy )
        strcpy(source, storage);
}
