#include "common.h"

/*字符串调整函数:
 * 向每超过一定长度的字符串中添加回车字符,避免单行过长
 *
 * p[]指针数组，指向若干个待处理字符串
 * len 为单行所需截取长度
 * storage[] 指针数组，用来存储处理后的字符串
 */
void adjustStr(char *p[], int len, char *storage[]) {

    printf("\nIn adjustStr fucntioin, test array address=%p\n\n", storage);

    for ( int i=0; i<3; i++ ) 
    {
        for ( int j=0, k=0; ; j++, k++ ) 
        {
            storage[i][k] = p[i][j];

            /*读到结尾字符时退出内层for循环，处理下一个字符串*/
            if ( p[i][j] == '\0' )
                break;

            /*因为含中文字符的字符串长度到该截取时不一定刚刚好是求余为0
             * 所以给个阈值，让字符串一定能在某个长度范围内被截取, && j
             * 作用是防止第一个字符就被截取掉*/
            if ( abs( len - (j % len) ) <= 3 && j)
            {
                /*此处需要了解一下utf8编码格式
                 *
                 * 对于只有一个字节的字符,单字节最高位为0，
                 * 否则最高位开始往后数，有多少个1，就代表
                 * 当前字符有多少字节，utf-8在本机测试为3个字节，
                 * utf-8格式为111xxx 10xxxx 10xxxx(二进制表示).
                 * 所以只要判断到当前字节的高两位为10,即十进制为2，且下一个字节
                 * 的高两位不是10，即不是十进制中的2，说明单个汉字已经读取结束
                 * 这个时候才可以对该字符后进行截取(加回车符),不然可能导致乱码*/
                if ( (((p[i][j] & 0xff) >> 6) & 0x03) == 2  
                        && (((p[i][j+1] & 0xff) >> 6) & 0x03) != 2 )

                    storage[i][++k] = '\n';
            }

            if ( j % len == 0 && j)
                if ( (p[i][j]&0xff) < 128 && (p[i][j]&&0xff >= 0) ) 
                    storage[i][++k] = '\n';
        }
    }

    printf("\nOut adjustStr\n\n");
}
