#include "common.h"
#include <ctype.h>

/*字符串调整函数:
 * 向每超过一定长度的字符串中添加回车字符,避免单行过长
 *
 * p[]指针数组，指向若干个待处理字符串
 * len 为单行所需截取长度
 * storage[] 指针数组，用来存储处理后的字符串
 */
void adjustStr(char *p[], int len, char *storage[]) {

    int nowlen = 0;
    int asciich = 0;
    int cansplit = 0;

    for ( int i=0; i<3; i++ ) 
    {
        nowlen = 0;

        for ( int j=0, k=0; True ; j++, k++ ) 
        {
            storage[i][k] = p[i][j];
            //printf("i=%d %c\n",i, p[i][j]);

            /*读到结尾字符时退出内层for循环，处理下一个字符串*/
            if ( p[i][j] == '\0' ) {
                if ( j != 0 && i != 0) {
                    strcat ( storage[i], "\n" );
                }
                break;
            }

            if ( p[i][j] == ' ')
                cansplit = k;

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
            if ( (((p[i][j] & 0xff) >> 6) & 0x03) == 2  
                    && (((p[i][j+1] & 0xff) >> 6) & 0x03) != 2 ) {

                nowlen++;

                if ( k > cansplit )
                    cansplit = k;
            }

            if ( (p[i][j]&0xff) < 128 && (p[i][j]&&0xff >= 0) )
                asciich++;

            if ( asciich == 2 ) {
                nowlen++;
                asciich = 0;
            }

            /* 防止单词被割裂为两行(上一行末尾 下一行开头) */ 
            if (nowlen == len && isalnum(p[i][j]) && isalnum(p[i][j+1]))
            {
                int count = k - cansplit;

                /* 排除可分割下标在行头或者待回退字符过长,此种情况强制切割*/
                if (! (count > len || k == 0)) {

                    int goback = 0;
                    while (isalnum(p[i][j--]))
                    {
                        storage[i][k - goback++] = ' ';
                    }
                }
            }

            if ( nowlen == len ) {

                storage[i][++k] = '\n';
                nowlen = 0;
                continue;
            }
        }
    }
}
