#include "common.h"

extern char *shmaddr_baidu;
extern char *shmaddr_google;

extern char *baidu_result[BAIDUSIZE] ;
extern char *google_result[GOOGLESIZE] ;

extern char *baidu_result[BAIDUSIZE];

extern char *text;

char audio_en[512] = { '\0' };
char audio_uk[512] = { '\0' };

void separateDataForBaidu(int *index, int len) {

    if ( index[0] == 0)
        return;

    if ( baidu_result[0] == NULL)
        err_exit("Doesn't init memory yet\n");

    /* TODO:去除SourceInput末尾的回车符后，比较新输入的字符串
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

        if ( strcmp ( tmp, SourceInput ) != 0 ) {

            strcat ( SourceInput, tmp );
            //strcat ( SourceInput, "\n" );
            adjustStrForBaidu(len, SourceInput, 0, 0);
        }
    }

    /*分离相应数据到特定功能内存区域*/
    for ( int n=1, i=0, count=0; n<=BAIDUSIZE; n++ ) {

        /*shmaddr_baidu[] 中0~BAIDUSIZE各字节的含义见gif_pic/protocol.png*/
        if ( shmaddr_baidu[n] - '0' == 0) 
        {
            i++;
            continue;
        }

        /* 提取音频链接*/
        if ( n == 5 ) {

            strcat ( audio_en, &shmaddr_baidu[index[i++]] );
            strcat ( audio_uk, &shmaddr_baidu[index[i++]] );
            continue;
        }

        count = 0;
        while ( count <  shmaddr_baidu[n] - '0') 
        {

            strcat ( baidu_result[n],  &shmaddr_baidu[index[i++]]);
            strcat ( baidu_result[n], "\n");

            /* 最后一条中文翻译前的解释都加上回车*/
            if ( n == 2 && count + 1 < shmaddr_baidu[n] - '0') {
                strcat ( ZhTrans, "\n");
            }

            count++;
        }

        /* ZhTrans*/
        if ( n == 2 ) {
            /*单个翻译结果不能加空格前缀 addSpace=0*/
            if ( PhoneticFlag == 0 && NumZhTranFlag == 1 && NumEnTranFlag == 0 && OtherWordFormFlag == 0 )
                adjustStrForBaidu(len, baidu_result[n], 0, 1);
            else
                adjustStrForBaidu(len, baidu_result[n], 1, 1);
        }

        else if ( n == 3 || n == 4)
            adjustStrForBaidu(len, baidu_result[n], 0, 1);

        /* 这条语句貌似是统计行数用的 */
        else if ( n == 1 )
            adjustStrForBaidu(len, baidu_result[n], 0, 0);
    }

    /*打印提取结果*/
    for ( int i=0; i<BAIDUSIZE; i++ )
        printf("\033[0;35mbaidu_result[%d]=%s\033[0m\n", i, baidu_result[i]);

    printf("\033[0;35maudio_en= %s \033[0m\n", audio_en);
    printf("\033[0;35maudio_uk= %s \033[0m\n", audio_uk);
}

void adjustStrForBaidu(int len, char *source, int addSpace, int copy) {

    //printf("In adjustStrForBaidu\n");

    int nowlen = 0;
    int asciich = 0;
    int prefixLen = 0;
    int start = 0;
    int validDot = 1;

    long int srclen = 0;
    srclen = strlen(source);
    //fprintf(stdout, "\033[0;31m srclen=%ld\033[0m\n",  srclen );

    if ( srclen > 1024 * 1024 ) {

        fprintf(stderr, "\033[0;31m\n源数据过长，临时数组无法容纳，"\
                "待处理此类情况..., 准备退出...\033[0m\n\n");
        quit();
    }

    /*Be careful here which might be cause stack overflow*/
    char storage[1024*1024] = { '\0' };

    for ( int j=0, k=0; True ; j++, k++ ) 
    {
        storage[k] = source[j];

        if ( source[j] == '\0' )
            break;


        if ( source[j] == '\n') {

            validDot = 1;
            start = j;
            nowlen = 0;
            continue;
        }

        /*语句末有一个结束符号也是点，不是有效点，不能进这个if执行逻辑*/
        if (addSpace && validDot && source[j] == '.' \
                && source[j+1] != '\n' && source[j+1] != ' ' ) {

            prefixLen = (j+1-start)*2;
            //printf("\033[0;34mj=%d start=%d prefixLen=%d\033[0m\n", j, start, prefixLen);
            nowlen = (j-start)/2;
            validDot = 0;

            //printf("\033[0;32m j=%d start=%d nowlen=%d \033[0m\n\n",j, start, nowlen);
        }

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
            //printf("\033[0;32mAdd Enter\033[0m\n");
            storage[++k] = '\n';
            nowlen = 0;

            if ( addSpace ) {
                int tmp = prefixLen;
                while(tmp--) {
                    storage[++k] = ' ';
                    nowlen++;
                }
                nowlen /= 2;
                //printf("\033[0;32m  (in if)nowlen=%d \033[0m\n\n", nowlen);
            }
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
        printf("\033[0;31m索引首值为0，无数据(Google in separateGoogleDataSetWinSize) \033[0m\n");
        return;
    }

    /* TODO:原来是全局变量*/
    int lines = 0;
    int maxlen = 0;

    /*主要完成步骤:加入回车符使单行句子不至于太长*/
    if ( shmaddr_google[0]  != ERRCHAR ) {

        int ret = 0;

        char *p[3];
        p[0] = &shmaddr_google[ACTUALSTART];
        p[1] = &shmaddr_google[index[0]];
        p[2] = &shmaddr_google[index[1]];

        /* TODO:上下代码位置调换过，出错回来这里*/
        adjustStr(p, 28, google_result);

        /* 统计谷歌翻译结果行数*/
        lines = getLinesOfGoogleTrans ( index );
        
        for ( int i=0; i<3; i++ )
            if ( ( ret = countCharNums ( google_result[i] ) ) > maxlen )
                maxlen = ret;

        if ( maxlen > 28 )
            maxlen = 28;

        printf("\033[0;33m(谷歌数据分离窗口调整) maxlen=%d lines=%d\033[0m\n\n", maxlen,lines);

        /*存于全局变量*/
        gw.width = 15 * maxlen;
        gw.height = lines * 28;

        for ( int i=0; i<3; i++ )
            printf("\033[0;33m(separateGoogleDataSetWinSize) %s \033[0m\n", google_result[i]);

        printf("\033[0;33mgw.width=%f, gw.height=%f \033[0m\n", gw.width, gw.height);
    }
    else  {
        shmaddr_google[0] = CLEAR;
        strcpy(google_result[0], "翻译超时或出现其他错误|");
        lines = 2;

        gw.width = 250;
        gw.height = gw.width * 0.618;
        maxlen = 12;
    }
}

