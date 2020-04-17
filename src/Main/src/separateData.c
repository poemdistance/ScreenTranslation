#include "common.h"
#include "newWindow.h"
#include "cleanup.h"

void separateDataForBaidu(int *index, int len, int type, WinData *wd) {

    AudioData *ad = wd->ad;
    MemoryData *med = wd->arg->med;

    char ***result = NULL;
    char *tmpBuffer = NULL;
    int enterNum = 0;
    int k = 0;
    tmpBuffer = med->tmp;
    enterNum = 0;

    /* 根据类型设置操作地址*/
    if ( type == ONLINE )
        result = med->bing_result;
    else if ( type ==  OFFLINE )
        result = med->mysql_result;

    clearBingMysqlResultMemory ( med->bing_result,  med->mysql_result);

    if ( index[0] == 0) return;

    if ( result[0] == NULL)
        err_exit("Doesn't init memory yet\n");

    if ( strlen ( med->text ) < 130 ) {

        char tmp[130];
        char *p = tmp;

        strcpy ( tmp, med->text ); /* med->text含回车符*/

        /* 去除回车符*/
        while ( *p ) {
            if ( *p == '\n' )
                *p = '\0';
            p++;
        }

        if ( strcmp ( tmp, SourceInput(med, type) ) != 0 ) {

            strcat ( SourceInput(med, type), tmp );
            adjustStrForBaidu(len, SourceInput(med, type), 0, 0, &enterNum);
        }
    }

    /*分离相应数据到特定功能内存区域*/
    for ( int n=1, i=0, count=0; n<=BING_SIZE; n++ ) {

        /*shmaddr_bing[] 中0~BING_SIZE各字节的含义见gif_pic/protocol.png*/
        if ( tmpBuffer[n] - '0' == 0) 
        {
            i++;
            continue;
        }

        /* 提取音频链接*/
        if ( n == 5 ) {

            strcpy ( AUDIO_AM(ad, type), &tmpBuffer[index[i++]] );
            pbgreen("copy audio:%s", AUDIO_AM(ad, type));
            strcpy ( AUDIO_EN(ad, type), &tmpBuffer[index[i++]] );
            pbgreen("copy audio:%s", AUDIO_EN(ad, type));
            continue;
        }

        k=0;
        count = 0;
        /* 从python端来的数据缺少回车符，将在这里加上*/
        while ( count <  tmpBuffer[n] - '0') 
        {
            strcat ( result[n][k],  &tmpBuffer[index[i++]]);
            strcat ( result[n][k], "\n");
            count++;

            /* n为2,3分别对应中英文翻译，一条结果用单独一个空间存储，故k++,
             * 除此外，其他结果只有一个存储空间，k不可超过0*/
            if ( n == 2 || n == 3 ) k++;
        }

        for ( int i=0; n==2 && i<ZH_EN_TRAN_SIZE && result[n][i][0]; i++ )
            adjustStrForBaidu(len, result[n][i], 0, 1, &enterNum);

        /* 处理英译英的结果*/
        for ( int i=0; n==3 && i<ZH_EN_TRAN_SIZE && result[n][i][0]; i++ )
            adjustStrForBaidu(len, result[n][i], 0, 1, &enterNum);

        if ( n == 4 )
            adjustStrForBaidu(len, result[n][0], 0, 1, &enterNum);
    }

    /* 回车符太多的话会导致窗口高度过大，跟宽度不协调。
     * 这里以8个回车符为界限，如果超过则适当增加每行
     * 显示的计数长度以此减少行数降低窗口高度.*/
    if ( enterNum > 8 ) 
        separateDataForBaidu ( index, len+8, type, wd );
}


void separateGoogleData ( int *index, int len, WinData *wd ) {

    int enterNum = 0;
    ShmData *sd = wd->sd;
    MemoryData *med = wd->arg->med;

    if ( index[0] == 0 ) {
        pred("索引首值为0，无数据 (separateGoogleData)");
        return;
    }

    /*主要完成步骤:加入回车符使单行句子不至于太长*/
    if ( sd->shmaddr_google[0]  != ERRCHAR ) {

        char *p[3];
        p[0] = &med->tmp[ACTUALSTART];
        p[1] = index[0] != 0 ? &med->tmp[index[0]] : NULL;
        p[2] = index[1] != 0 ? &med->tmp[index[1]] : NULL;

        adjustStrForGoogle(p, len, med->google_result, &enterNum);
    }
    else  {
        sd->shmaddr_google[0] = CLEAR;
        strcpy(med->google_result[0], "翻译超时或出现其他错误");
    }

    if ( enterNum > 8 && len < 100 )
        separateGoogleData ( index, len+8, wd );
}

