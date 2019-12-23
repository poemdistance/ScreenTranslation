#include "common.h"
#include "newWindow.h"
#include "windowData.h"
#include "dataStatistics.h"

int getMaxLenOfBaiduTrans(int type) {

    char ***result = NULL;
    if ( type == ONLINE )
        result = baidu_result;
    else if ( type == OFFLINE ) 
        result = mysql_result;

    int maxlen = 0;

    for ( int i=0, len=0; i<BAIDUSIZE; i++ ) {

        if ( i == 2 || i == 3 ) {
            for ( int j=0; j<ZH_EN_TRAN_SIZE; j++ ) {
                len = countCharNums ( result[i][j] );
                maxlen = len > maxlen ? len : maxlen;
            }
        }
        else {
            len = countCharNums ( result[i][0] );
            maxlen = len > maxlen ? len : maxlen;
        }
    }

    if ( maxlen > 28 )
        maxlen = 28;

    return maxlen;
}

int getLinesNumOfBaiduTrans (int type) {

    char ***result = NULL;
    if ( type == ONLINE )
        result = baidu_result;
    else if ( type == OFFLINE )
        result = mysql_result;

    int resultNum = 0;

    /* lines起始为1，因为源数据没有被插入回车符，后面计算不到，这里
     * 手动加1*/
    int lines = 1;

    char *p = NULL;

    for ( int i=0; i<BAIDUSIZE; i++ ) {

        if ( i == 2 || i == 3 ) {
            for ( int j=0; j<ZH_EN_TRAN_SIZE; j++ ) {
                if ( result[i][j][0]) {
                    resultNum++;
                    p = result[i][j];
                    while ( *p ) {
                        if ( *p++ == '\n' )
                            lines++;
                    }
                }
            }
        }
        else {
            if ( result[i][0][0] != '\0' ) {
                resultNum++;
                p = result[i][0];
                while ( *p ) {
                    if ( *p++ == '\n' )
                        lines++;
                }
            }
        }
    }

    /* 算上到时插入到显示界面的空行，最后一个结果后面不插入空行，
     * 所以减1*/
    lines = lines + resultNum - 1;

    return lines;
}

int getLinesNumOfGoogleTrans () {

    int lines = 0;
    char *p = NULL;

    if ( strlen(&shmaddr_google[ACTUALSTART]) < 30 )
        lines++;

    for ( int i=0; i<3; i++ ) {
        if ( strlen ( google_result[i] ) > 0 ) {

            /* 前两个数据不为空，后续会增加空行，这里一并计算*/
            if ( i < 2 )
                lines++;

            p = google_result[i];
            while ( *p ) {
                if ( *p++ == '\n' )
                    lines++;
            }
        }
    }

    return lines;
}

int getMaxLenOfGoogleTrans() {

    int ret = 0;
    int maxlen = 0;
    for ( int i=0; i<3; i++ ) 
        if ( ( ret = countCharNums ( google_result[i] ) ) > maxlen )
            maxlen = ret;

    return maxlen < 28 ? maxlen : 28;
}
