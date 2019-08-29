#include "common.h"

extern char *shmaddr_baidu;

extern char *baidu_result[BAIDUSIZE];

extern char *text;

int lines_baidu = 0;
int maxlen_baidu = 0;

char audio_en[512] = { '\0' };
char audio_uk[512] = { '\0' };

void separateData(int *index, int len) {

    if ( index[0] == 0)
        return;

    lines_baidu = 0;

    if ( baidu_result[0] == NULL)
        err_exit("Doesn't init memory yet\n");

    /* TODO:去除SourceInput末尾的回车符后，比较新输入的字符串
     * 是否与之相等，相等说明复制过了，取消本次复制 ( <130的判断
     * 是因为>=130时, 原始字符是不显示的, 没必要复制)*/
    if ( strlen ( text ) < 130 ) {

        char tmp[130];
        strcpy ( tmp, text );
        strcat ( tmp, "\n" );

        if ( strcmp ( tmp, SourceInput ) != 0 ) {

            strcat ( SourceInput, text );
            strcat ( SourceInput, "\n" );
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

            if ( n == 2 && count + 1 < shmaddr_baidu[n] - '0') {
                strcat ( baidu_result[n], "\n");
            }

            count++;
        }

        if ( n == 2 ) {
            /*单个翻译结果不能加空格前缀 addSpace=0*/
            if ( PhoneticFlag == 0 && NumZhTranFlag == 1 && NumEnTranFlag == 0 && OtherWordFormFlag == 0 )
                adjustStrForBaidu(len, baidu_result[n], 0, 1);
            else
                adjustStrForBaidu(len, baidu_result[n], 1, 1);
        }

        else if ( n == 3 || n == 4)
            adjustStrForBaidu(len, baidu_result[n], 0, 1);
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

            lines_baidu++;
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

        //printf("\033[0;32m  nowlen=%d \033[0m\n\n", nowlen);
        if (nowlen && nowlen > maxlen_baidu )
            maxlen_baidu = nowlen;

        if ( nowlen == len ) {
            //printf("\033[0;32mAdd Enter\033[0m\n");
            storage[++k] = '\n';
            lines_baidu++;
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

    if ( maxlen_baidu > 28 )
        fprintf(stderr, "\033[0;31m maxlen_baidu=%d 超过预期值\033[0m\n\n", maxlen_baidu);
}
