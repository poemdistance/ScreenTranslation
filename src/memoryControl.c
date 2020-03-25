#include "common.h"
#include "newWindow.h"
#include "cleanup.h"
#include <assert.h>

extern char audioOnline_en[512];
extern char audioOnline_uk[512];

extern char audioOffline_en[512];
extern char audioOffline_uk[512];

extern volatile sig_atomic_t InNewWin;

/* 初始化离线翻译结果存储空间*/
void initMemoryMysql() {

    if (mysql_result[0] != NULL)
        return;

    for (int i=0; i<MYSQLSIZE; i++) {

        if ( i == 2  || i == 3 ) {

            mysql_result[i] = calloc(ZH_EN_TRAN_SIZE, sizeof ( char* ) );
            assert(mysql_result[i] != NULL);

            for ( int j=0; j<ZH_EN_TRAN_SIZE; j++ ) {
                mysql_result[i][j] = calloc ( PER_SENTENCE_SIZE, sizeof(char) );
                assert(mysql_result[i][j] != NULL);
            }
            continue;
        }
        mysql_result[i] = calloc(1, sizeof ( char* ) );
        assert(mysql_result[i] != NULL);

        mysql_result[i][0] = calloc ( SHMSIZE / MYSQLSIZE, sizeof(char) );
        assert(mysql_result[i][0] != NULL);
    }
}

void releaseMemoryMysql() {

    /* 释放翻译结果存储空间 <Mysql>*/
    if ( mysql_result[0] != NULL)
        for (int i=0; i<MYSQLSIZE; i++) {

            if ( i == 2 || i == 3 ) {
                for ( int j=0; j<ZH_EN_TRAN_SIZE; j++ )
                    if ( mysql_result[i][j] ) 
                        free ( mysql_result[i][j] );
            }
            else {

                if ( mysql_result[i][0] )
                    free ( mysql_result[i][0] );
            }

            if ( mysql_result[i] )
                free ( mysql_result[i] );
        }
}

void initMemoryTmp() {

    if (tmp != NULL)
        return;

    tmp = calloc ( SHMSIZE , sizeof(char) );
}

void releaseMemoryTmp() {

    /* 临时缓冲区释放*/
    if ( tmp != NULL )
        free ( tmp );
}

void releaseMemoryBaidu() {

    /* 释放翻译结果存储空间 <Mysql>*/
    if ( baidu_result[0] != NULL)
        for (int i=0; i<MYSQLSIZE; i++) {

            if ( i == 2 || i == 3 ) {
                for ( int j=0; j<ZH_EN_TRAN_SIZE; j++ )
                    if ( baidu_result[i][j] ) 
                        free ( baidu_result[i][j] );
            }
            else {

                if ( baidu_result[i][0] )
                    free ( baidu_result[i][0] );
            }

            if ( baidu_result[i] )
                free ( baidu_result[i] );
        }
}

/* 初始化百度翻译结果存储空间*/
void initMemoryBaidu() {

    if (baidu_result[0] != NULL)
        return;

    for (int i=0; i<BAIDUSIZE; i++) {

        if ( i == 2  || i == 3 ) {

            baidu_result[i] = calloc(ZH_EN_TRAN_SIZE, sizeof ( char* ) );
            assert(baidu_result[i] != NULL);

            for ( int j=0; j<ZH_EN_TRAN_SIZE; j++ ) {
                baidu_result[i][j] = calloc ( PER_SENTENCE_SIZE, sizeof(char) );
                assert(baidu_result[i][j] != NULL);
            }
            continue;
        }
        baidu_result[i] = calloc(1, sizeof ( char* ) );
        assert(baidu_result[i] != NULL);

        baidu_result[i][0] = calloc ( SHMSIZE / BAIDUSIZE, sizeof(char) );
        assert(baidu_result[i][0] != NULL);
    }

}

/* 类上*/
void initMemoryGoogle() {

    if (google_result[0] != NULL)
        return;

    for (int i=0; i<GOOGLESIZE; i++) {

        google_result[i] = calloc(SHMSIZE / GOOGLESIZE, sizeof ( char ) );
        if (google_result[i] == NULL)
            err_exit("Error occured when calloc memory in initMemoryGoogle");
    }
}

void releaseMemoryGoogle() {

    /* 释放翻译结果存储空间 <Google>*/
    if ( google_result[0] != NULL)
        for (int i=0; i<GOOGLESIZE; i++)
            free(google_result[i]);
}

void clearBaiduMysqlResultMemory() {

    if ( ! baidu_result[0] )
        return;

    for ( int i=0; i<BAIDUSIZE; i++ ) {
        if ( i == 2 || i == 3 ) {
            for ( int j=0; j<ZH_EN_TRAN_SIZE; j++ ) {
                if ( baidu_result[i] && baidu_result[i][j] )
                    memset ( baidu_result[i][j], '\0',  PER_SENTENCE_SIZE );
                if ( mysql_result[i] && mysql_result[i][j] )
                    memset ( mysql_result[i][j], '\0',  PER_SENTENCE_SIZE );
            }
        }
        else {
            if ( baidu_result[i] && baidu_result[i][0] ) {
                memset ( baidu_result[i][0], '\0', SHMSIZE / BAIDUSIZE );
            }
            if ( mysql_result[i] && mysql_result[i][0] ) {

                memset ( mysql_result[i][0], '\0', SHMSIZE / MYSQLSIZE );
            }
        }
    }
}

void clearMemory () {

    if ( tmp ) {
        memset ( tmp, '0', 10 );
        memset ( &tmp[10], '\0', SHMSIZE-10);
    }

    /* 标志位空间用字符0填充*/
    memset(shmaddr_baidu, '0', 10);
    memset(shmaddr_mysql, '0', 10);
    memset(shmaddr_pic, '0', 10);
    memset(shmaddr_keyboard, '0', 10);
    memset(shmaddr_google, '0', 10);

    memset(&shmaddr_google[10], '\0', SHMSIZE-10);
    memset(&shmaddr_baidu[10], '\0', SHMSIZE-10);
    memset(&shmaddr_mysql[10], '\0', SHMSIZE-10);
    memset(&shmaddr_pic[10], '\0', SHMSIZE-10);

    memset ( audio_en(ONLINE), '\0', 512 );
    memset ( audio_uk(ONLINE), '\0', 512 );
    memset ( audio_en(OFFLINE), '\0', 512 );
    memset ( audio_uk(OFFLINE), '\0', 512 );

    clearBaiduMysqlResultMemory();

    for ( int i=0; i<GOOGLESIZE; i++ )
        if ( google_result[i] != NULL)
            memset( google_result[i], '\0', SHMSIZE / GOOGLESIZE );

    InNewWin = 0;
}
