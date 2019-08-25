#include "common.h"

extern char *shmaddr_baidu;

#define PhoneticFlag ( shmaddr_baidu[1] - '0' )
#define NumZhTranFlag ( shmaddr_baidu[2] - '0')
#define NumEnTranFlag ( shmaddr_baidu[3] - '0')
#define OtherWordFormFlag ( shmaddr_baidu[4] - '0')
#define NumAudioFlag ( shmaddr_baidu[5] - '0')


extern char *baidu_result[BAIDUSIZE];

#define SourceInput ((char *)( baidu_result[0] ))
#define Phonetic ((char *)( baidu_result[1] ))
#define ZhTrans ((char *)( baidu_result[2] ))
#define EnTrans ((char *)( baidu_result[3] ))
#define OtherWordForm ((char *)( baidu_result[4] ))
#define Audios ((char *)( baidu_result[5] ))

extern char *text;

void adjustStrForBaidu(int len, char *source, int addSpace, int copy);

int lines_baidu = 0;
int maxlen_baidu = 0;

void separateData(int *index, int len) {

    lines_baidu = 0;

    if ( baidu_result[0] == NULL)
        err_exit("Doesn't init memory yet\n");

    strcat ( SourceInput, text );
    strcat ( SourceInput, "\n" );
    adjustStrForBaidu(len, SourceInput, 0, 0);

    /*分离相应数据到特定功能内存区域*/
    for ( int n=1, i=0, count=0; n<=BAIDUSIZE; n++ ) {

        /*shmaddr_baidu[] 中0~BAIDUSIZE各字节的含义见gif_pic/protocol.png*/
        if ( shmaddr_baidu[n] - '0' == 0) 
        {
            i++;
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

    for ( int i=0; i<BAIDUSIZE; i++ )
        printf("%s\n", baidu_result[i]);
}

void adjustStrForBaidu(int len, char *source, int addSpace, int copy) {

    //printf("In adjustStrForBaidu\n");

    int nowlen = 0;
    int asciich = 0;
    int prefixLen = 0;
    int start = 0;
    int validDot = 1;

    char storage[4096*2] = { '\0' };

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

        if (addSpace && validDot && source[j] == '.' && source[j+1] != '\n') {

            prefixLen = (j+1-start)*2;
            nowlen = (j-start)/2;
            validDot = 0;
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

        if (nowlen && nowlen > maxlen_baidu ) {

            maxlen_baidu = nowlen;
        }
        //printf("nowlen=%d\n", nowlen);
        if ( nowlen == len ) {
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
            }
        }
    }

    if ( copy )
        strcpy(source, storage);

    //printf("maxlen_baidu=%d\n", maxlen_baidu);
    //printf("Out adjustStrForBaidu\n");
}
