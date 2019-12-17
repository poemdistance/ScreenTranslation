#include "common.h"
#include "newWindow.h"
#include "cleanup.h"

extern char audioOnline_en[512];
extern char audioOnline_uk[512];

extern char audioOffline_en[512];
extern char audioOffline_uk[512];

/* 初始化离线翻译结果存储空间*/
void initMemoryMysql() {

    if (mysql_result[0] != NULL)
        return;

    for (int i=0; i<MYSQLSIZE; i++) {

        mysql_result[i] = calloc(SHMSIZE / MYSQLSIZE, sizeof ( char ) );
        if (mysql_result[i] == NULL)
            err_exit("Error occured when calloc memory in initMemoryMysql");
    }
}

/* 初始化百度翻译结果存储空间*/
void initMemoryBaidu() {

    if (baidu_result[0] != NULL)
        return;

    for (int i=0; i<BAIDUSIZE; i++) {

        baidu_result[i] = calloc(SHMSIZE / BAIDUSIZE, sizeof ( char ) );
        if (baidu_result[i] == NULL)
            err_exit("Error occured when calloc memory in initMemoryBaidu");
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

void clearMemory () {

    /* 标志位空间用字符0填充*/
    memset(shmaddr_baidu, '0', 10);
    memset(shmaddr_mysql, '0', 10);
    memset(shmaddr_pic, '0', 10);
    memset(shmaddr_keyboard, '0', 10);

    memset(shmaddr_google, '\0', SHMSIZE);
    memset(shmaddr_baidu, '\0', SHMSIZE-10);
    memset(shmaddr_mysql, '\0', SHMSIZE-10);
    memset(shmaddr_pic, '\0', SHMSIZE-10);

    memset ( audio_en(ONLINE), '\0', 512 );
    memset ( audio_uk(ONLINE), '\0', 512 );
    memset ( audio_en(OFFLINE), '\0', 512 );
    memset ( audio_uk(OFFLINE), '\0', 512 );

    for ( int i=0; i<BAIDUSIZE; i++ )
        if ( baidu_result[i] != NULL)
            memset( baidu_result[i], '\0', SHMSIZE / BAIDUSIZE );

    for ( int i=0; i<MYSQLSIZE; i++ )
        if ( mysql_result[i] != NULL)
            memset( mysql_result[i], '\0', SHMSIZE / MYSQLSIZE );

    for ( int i=0; i<GOOGLESIZE; i++ )
        if ( google_result[i] != NULL)
            memset( google_result[i], '\0', SHMSIZE / GOOGLESIZE );
}
