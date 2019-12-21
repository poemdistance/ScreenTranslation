#include "expanduser.h"
#include "printWithColor.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define STRING "audioButtonPosition"

/* 函数功能: 替换字符串中的$USE为系统登录用户名
 *
 * 函数返回替换后的静态存储区的字符串，共维护有
 * 1024条字符串，最长支持字符长度1024，如果计算
 * 到此次替换后的结果已经存在，那么将返回已有的
 * 结果,否则将此结果添加到维护字符串中并返回该
 * 结果*/

char *expanduser ( const char *str ) {

    static char result[1024] = { '\0' };
    static char storage[1024][1024] = { '\0' };

    strcpy ( result, str );
    char *p = NULL;
    if ( ! (  p = strstr ( result, "$USER" ) ) )
        return p;

    char tmp[1024] = { '\0' };
    strcpy ( tmp, p+sizeof("$USER")-1 );

    *p = '\0';
    strcat ( strcat ( (char*)result, getlogin()), tmp);

    for ( int i=0; i<1024; i++ ) {

        if ( strcmp ( result, (char*)&storage[i] ) == 0 )
            return (char*)&storage[i];
        else if ( ! storage[i][0] )
            return strcpy ( (char*)&storage[i], result );
    }

    pbred ( "所维护字符串条数已超过限制- 1024" );
    pbred ( "注意之后已有的返回结果将受后续调用的影响" );

    return result;
}

//int main() {
//
//    puts ( expanduser ( "/home/$USER/.stran/"STRING".func" ) );
//    return 0;
//}
