#include "common.h"
#include "newWindow.h"
#include "cleanup.h"

extern char *shmaddr_baidu;
extern char *shmaddr_google;
extern char *shmaddr_mysql;

extern char *baidu_result[BAIDUSIZE] ;
extern char *google_result[GOOGLESIZE] ;
extern char *mysql_result[MYSQLSIZE] ;

extern char *baidu_result[BAIDUSIZE];

extern char *text;

char audioOnline_en[512] = { '\0' };
char audioOnline_uk[512] = { '\0' };

char audioOffline_en[512] = { '\0' };
char audioOffline_uk[512] = { '\0' };

void separateDataForBaidu(int *index, int len, int type) {

    char **result = NULL;
    char *shmaddr = NULL;

    /* 根据类型设置操作地址*/
    if ( type == ONLINE ) {

        result = baidu_result;
        shmaddr = shmaddr_baidu;
    }
    else if ( type ==  OFFLINE ) {

        result = mysql_result;
        shmaddr = shmaddr_mysql;
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
        if ( shmaddr[n] - '0' == 0) 
        {
            i++;
            continue;
        }

        /* 提取音频链接*/
        if ( n == 5 ) {

            strcat ( audio_en(type), &shmaddr[index[i++]] );
            strcat ( audio_uk(type), &shmaddr[index[i++]] );
            continue;
        }

        count = 0;
        while ( count <  shmaddr[n] - '0') 
        {
            strcat ( result[n],  &shmaddr[index[i++]]);
            strcat ( result[n], "\n");

            /* 最后一条中文翻译前的解释都加上回车*/
            if ( n == 2 && count + 1 < shmaddr[n] - '0') {
                strcat ( ZhTrans(type), "\n");
            }

            count++;
        }

        /* ZhTrans(type)*/
        if ( n == 2 ) {
            /*单个翻译结果不能加空格前缀 addSpace=0*/
            if ( PhoneticFlag(type) == 0 && NumZhTranFlag(type) == 1 &&\
                    NumEnTranFlag(type) == 0 && OtherWordFormFlag(type) == 0 ) {

                adjustStrForBaidu(len, result[n], 0, 1);
            }

            else
                adjustStrForBaidu(len, result[n], 1, 1);
        }

        else if ( n == 3 || n == 4)
            adjustStrForBaidu(len, result[n], 0, 1);
    }
}

void adjustStrForBaidu(int len, char *source, int addSpace, int copy) {

    int nowlen = 0;
    int asciich = 0;
    int prefixLen = 0;
    int start = 0;
    int validDot = 1;
    int cansplit = 0;

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

        if ( source[j] == '\0' )
            break;

        /* 空格为可分割字符，记录下该下标*/
        if ( source[j] == ' ')
            cansplit = k;

        if ( source[j] == '\n') {

            validDot = 1;
            start = j;
            nowlen = 0;
            continue;
        }

        /*语句末有一个结束符号也是点，不是有效点，不能进这个if执行逻辑*/
        if (addSpace && validDot && source[j] == '.' \
                && source[j+1] != '\n' && source[j+1] != ' ' ) {

            /* 新前面应该加的空格长度，应连个ascii作一个字符长，因乘2,
             * start是上一个遇到的回车符的下标*/
            prefixLen = (j+1-start)*2;

            //printf("\033[0;34mj=%d start=%d prefixLen=%d\033[0m\n", j, start, prefixLen);
            nowlen = (j-start)/2;
            validDot = 0;

            //printf("\033[0;32m j=%d start=%d nowlen=%d \033[0m\n\n",j, start, nowlen);
        }

        /* 单个汉字的最后一个字节*/
        if ( (((source[j] & 0xff) >> 6) & 0x03) == 2  
                && (((source[j+1] & 0xff) >> 6) & 0x03) != 2 ) {

            nowlen++;

            /* 汉字可分割，如果下标大于上一个可分割空格字符的，则替换之*/
            if ( k > cansplit )
                cansplit = k;
        }

        /* ascii字符范围*/
        if ( (source[j]&0xff) < 128 && (source[j]&&0xff >= 0) )
            asciich++;

        /* 两个ascii码作一个字符长度*/
        if ( asciich == 2 ) {
            nowlen++;
            asciich = 0;
        }

        if ( nowlen == len ) {

            char nextchar = source[j+1];
            int forward = 0;
            int start = 0;
            int end = 0;

            /* 如果不是数字和字母，直接用回车符切割*/
            if ( ! (\
                        (nextchar >= '0' && nextchar <= '9' ) ||\
                        (nextchar >= 'a' && nextchar <= 'z')  ||\
                        (nextchar >= 'A' && nextchar <= 'Z')) ) {

                storage[++k] = '\n';

                nowlen = 0;
            } 
            else {

                /* 离上一个可分割字符的距离*/
                int count = k - cansplit;
                int right = k+1;
                int left = k;

                /* 如果上一个可分割字符在起点或者离上一个可分割字符超过一行的长度，那么强制分割，并返回for循环*/
                if ( count > len || cansplit == 0) {
                    storage[++k] = '\n';
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

                k++;
                start = left + 2;
                forward = 1;
            }

            /*pressing*/
            if ( addSpace ) {

                if ( forward == 1 )
                    k = start-1;

                int tmp = prefixLen;
                while(tmp--) {
                    storage[++k] = ' ';
                    nowlen++;
                }
                //nowlen /= 2;
                nowlen -= prefixLen/2;

                if ( forward == 1 )
                k = end;
            }
            continue;
        }
    }

    if ( copy )
        strcpy(source, storage);
}

int getLinesOfGoogleTrans ( int *index ) {

    int lines = 0;

    /*长度小于30会显示原始输入，lines加1*/
    if ( strlen(&shmaddr_google[ACTUALSTART]) < 30 ) {

        lines++;

        /* 无翻译结果*/
        if ( index[0] == 0 )
            return ++lines;
    }

    char *p = NULL;

    /* 显示时多了空行，index有值代表含有对应的结果，
     * 总共就是两行，lines+2*/
    for ( int i=0; i<3; i++ ) {

        /* index只有两个值，一个不为0，说明含对应数据，算上空行lines要自增*/
        if ( i < 2 && index[i] > 0 )
            lines++;

        p = google_result[i];
        while ( *p ) {
            if ( *p++ == '\n' )
                lines++;
        }
    }

    return lines;
}


void separateGoogleDataSetWinSize ( int *index ) {

    if ( index[0] == 0 ) {
        printf("\033[0;31m索引首值为0，无数据(separateGoogleDataSetWinSize) \033[0m\n");
        return;
    }

    int lines = 0;
    int maxlen = 0;

    /*主要完成步骤:加入回车符使单行句子不至于太长*/
    if ( shmaddr_google[0]  != ERRCHAR ) {

        int ret = 0;

        char *p[3];
        p[0] = &shmaddr_google[ACTUALSTART];
        p[1] = &shmaddr_google[index[0]];
        p[2] = &shmaddr_google[index[1]];

        adjustStr(p, 28, google_result);

        /* 统计谷歌翻译结果行数*/
        lines = getLinesOfGoogleTrans ( index );

        for ( int i=0; i<3; i++ )
            if ( ( ret = countCharNums ( google_result[i] ) ) > maxlen )
                maxlen = ret;

        if ( maxlen > 28 )
            maxlen = 28;

        pbyellow("(谷歌数据分离窗口调整) maxlen=%d lines=%d\n", maxlen,lines);

        /*存于全局变量*/
        gw.width = 15 * maxlen + 40;
        gw.height = lines * 28 + 45;
    }
    else  {
        shmaddr_google[0] = CLEAR;
        strcpy(google_result[0], "翻译超时或出现其他错误");
        lines = 2;

        gw.width = 250;
        gw.height = gw.width * 0.618 + 45;
        maxlen = 12;
    }

    gw.lines = lines;
    gw.maxlen = maxlen;
}

