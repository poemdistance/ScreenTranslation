#include "common.h"
#include "newWindow.h"
#include "cleanup.h"

extern char *shmaddr_baidu;
extern char *shmaddr_google;
extern char *shmaddr_mysql;

extern char **baidu_result[BAIDUSIZE] ;
extern char *google_result[GOOGLESIZE] ;
extern char **mysql_result[MYSQLSIZE] ;

extern char *text;

char audioOnline_en[512] = { '\0' };
char audioOnline_uk[512] = { '\0' };

char audioOffline_en[512] = { '\0' };
char audioOffline_uk[512] = { '\0' };

void separateDataForBaidu(int *index, int len, int type) {

    char ***result = NULL;
    char *tmpBuffer = NULL;

    /* 根据类型设置操作地址*/
    if ( type == ONLINE ) {

        result = baidu_result;
        tmpBuffer = tmp;
    }
    else if ( type ==  OFFLINE ) {

        result = mysql_result;
        tmpBuffer = tmp;
    }


    if ( index[0] == 0)
        return;

    if ( result[0] == NULL)
        err_exit("Doesn't init memory yet\n");

    /* TODO:去除SourceInput(type)末尾的回车符后，比较新输入的字符串
     * 是否与之相等，相等说明复制过了，取消本次复制 ( <130的判断
     * 是因为>=130时, 原始字符是不显示的, 没必要复制)*/
    if ( strlen ( text ) < 130 ) {

        char tmp[130];
        char *p = tmp;

        strcpy ( tmp, text ); /* text含回车符*/

        /* 去除回车符*/
        while ( *p ) {
            if ( *p == '\n' )
                *p = '\0';
            p++;
        }

        if ( strcmp ( tmp, SourceInput(type) ) != 0 ) {

            strcat ( SourceInput(type), tmp );
            adjustStrForBaidu(len, SourceInput(type), 0, 0);
        }
    }

    /*分离相应数据到特定功能内存区域*/
    for ( int n=1, i=0, count=0; n<=BAIDUSIZE; n++ ) {

        /*shmaddr_baidu[] 中0~BAIDUSIZE各字节的含义见gif_pic/protocol.png*/
        if ( tmpBuffer[n] - '0' == 0) 
        {
            i++;
            continue;
        }

        /* 提取音频链接*/
        if ( n == 5 ) {

            strcat ( audio_en(type), &tmpBuffer[index[i++]] );
            strcat ( audio_uk(type), &tmpBuffer[index[i++]] );
            continue;
        }

        int k=0;
        count = 0;
        /* 从python端来的数据缺少回车符，将在这里加上*/
        while ( count <  tmpBuffer[n] - '0') 
        {
            strcat ( result[n][k],  &tmpBuffer[index[i++]]);
            strcat ( result[n][k], "\n");

            /* 除去最后一条中文翻译，之前的字符串都再加上回车（形成空行,用于隔开各个句子）*/
            if ( n == 2 && count + 1 < tmpBuffer[n] - '0') {
                strcat ( ZhTrans(type, k), "\n");
            }

            count++;

            /* n为2,3分别对应中英文翻译，一条结果用单独一个空间存储，故i++,
             * 除此外，其他结果只有一个存储空间，i不可超过0*/
            if ( n == 2 || n == 3 )
                k++;
        }

        /* ZhTrans(type)*/
        if ( n == 2 ) {
            /*单个翻译结果不能加空格前缀 addSpace=0*/
            if ( PhoneticFlag(type) == 0 && NumZhTranFlag(type) == 1 &&\
                    NumEnTranFlag(type) == 0 && OtherWordFormFlag(type) == 0 ) {

                adjustStrForBaidu(len, result[n][0], 0, 1);
            }

            else {

                for ( int i=0; i<ZH_EN_TRAN_SIZE; i++ )
                    adjustStrForBaidu(len, result[n][i], 1, 1);
            }
        }

        else if ( n == 3 ) {

            for ( int i=0; i<ZH_EN_TRAN_SIZE; i++ )
                adjustStrForBaidu(len, result[n][i], 0, 1);
        }
        else if ( n == 4 )
            adjustStrForBaidu(len, result[n][0], 0, 1);
    }
}


void separateGoogleData ( int *index ) {

    if ( index[0] == 0 ) {
        printf("\033[0;31m索引首值为0，无数据(separateGoogleData) \033[0m\n");
        return;
    }

    /*主要完成步骤:加入回车符使单行句子不至于太长*/
    if ( shmaddr_google[0]  != ERRCHAR ) {

        char *p[3];
        p[0] = &tmp[ACTUALSTART];
        p[1] = index[0] != 0 ? &tmp[index[0]] : NULL;
        p[2] = index[1] != 0 ? &tmp[index[1]] : NULL;

        adjustStrForGoogle(p, 28, google_result);
    }
    else  {
        shmaddr_google[0] = CLEAR;
        strcpy(google_result[0], "翻译超时或出现其他错误");
    }
}

