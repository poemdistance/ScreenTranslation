#include "common.h"
#include "newWindow.h"
#include "cleanup.h"
#include <assert.h>

/* 初始化离线翻译结果存储空间*/
void initMemoryMysql ( char ***mysql_result ) {

    if (mysql_result[0] != NULL)
        return;

    for (int i=0; i<MYSQL_SIZE; i++) {

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

        mysql_result[i][0] = calloc ( SHMSIZE / MYSQL_SIZE, sizeof(char) );
        assert(mysql_result[i][0] != NULL);
    }
}

void releaseMemoryMysql ( char ***mysql_result ) {

    /* 释放翻译结果存储空间 <Mysql>*/
    if ( mysql_result[0] != NULL)
        for (int i=0; i<MYSQL_SIZE; i++) {

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

void initMemoryTmp ( char **tmp ) {

    pbmag ( "初始化翻译结果临时存储空间" );

    if ( *tmp != NULL) return;

    *tmp = calloc ( SHMSIZE , sizeof(char) );

    if ( *tmp )
        pbmag ( "翻译结果临时存储空间 初始化成功!" );
}

void releaseMemoryTmp ( char *tmp ) {

    /* 临时缓冲区释放*/
    if ( tmp != NULL )
        free ( tmp );
}

void releaseMemoryBing ( char ***bing_result ) {

    /* 释放翻译结果存储空间 <Mysql>*/
    if ( bing_result[0] != NULL)
        for (int i=0; i<MYSQL_SIZE; i++) {

            if ( i == 2 || i == 3 ) {
                for ( int j=0; j<ZH_EN_TRAN_SIZE; j++ )
                    if ( bing_result[i][j] ) 
                        free ( bing_result[i][j] );
            }
            else {

                if ( bing_result[i][0] )
                    free ( bing_result[i][0] );
            }

            if ( bing_result[i] )
                free ( bing_result[i] );
        }
}

/* 初始化百度翻译结果存储空间*/
void initMemoryBing ( char ***bing_result ) {

    if (bing_result[0] != NULL)
        return;

    for (int i=0; i<BING_SIZE; i++) {

        if ( i == 2  || i == 3 ) {

            bing_result[i] = calloc(ZH_EN_TRAN_SIZE, sizeof ( char* ) );
            assert(bing_result[i] != NULL);

            for ( int j=0; j<ZH_EN_TRAN_SIZE; j++ ) {
                bing_result[i][j] = calloc ( PER_SENTENCE_SIZE, sizeof(char) );
                assert(bing_result[i][j] != NULL);
            }
            continue;
        }
        bing_result[i] = calloc(1, sizeof ( char* ) );
        assert(bing_result[i] != NULL);

        bing_result[i][0] = calloc ( SHMSIZE / BING_SIZE, sizeof(char) );
        assert(bing_result[i][0] != NULL);
    }

}

/* 类上*/
void initMemoryGoogle ( char **google_result ) {

    if (google_result[0] != NULL)
        return;

    for (int i=0; i<GOOGLE_SIZE; i++) {

        google_result[i] = calloc(SHMSIZE / GOOGLE_SIZE, sizeof ( char ) );
        if (google_result[i] == NULL)
            err_exit("Error occured when calloc memory in initMemoryGoogle");
    }
}

void releaseMemoryGoogle ( char **google_result ) {

    /* 释放翻译结果存储空间 <Google>*/
    if ( google_result[0] != NULL)
        for (int i=0; i<GOOGLE_SIZE; i++)
            free(google_result[i]);
}

void clearBingMysqlResultMemory ( char ***bing_result, char ***mysql_result ) {

    if ( ! bing_result[0] )
        return;

    for ( int i=0; i<BING_SIZE; i++ ) {
        if ( i == 2 || i == 3 ) {
            for ( int j=0; j<ZH_EN_TRAN_SIZE; j++ ) {
                if ( bing_result[i] && bing_result[i][j] )
                    memset ( bing_result[i][j], '\0',  PER_SENTENCE_SIZE );
                if ( mysql_result[i] && mysql_result[i][j] )
                    memset ( mysql_result[i][j], '\0',  PER_SENTENCE_SIZE );
            }
        }
        else {
            if ( bing_result[i] && bing_result[i][0] ) {
                memset ( bing_result[i][0], '\0', SHMSIZE / BING_SIZE );
            }
            if ( mysql_result[i] && mysql_result[i][0] ) {

                memset ( mysql_result[i][0], '\0', SHMSIZE / MYSQL_SIZE );
            }
        }
    }
}

void clearMemory ( void *data ) {

    Arg         *arg = data;
    ShmData     *sd  = arg->sd;
    MemoryData  *med = arg->med;

    if ( med->tmp ) {
        memset ( med->tmp, '0', 10 );
        memset ( &med->tmp[10], '\0', SHMSIZE-10);
    }

    /* 标志位空间用字符0填充*/
    memset(sd->shmaddr_bing,    '0', 10);
    memset(sd->shmaddr_mysql,   '0', 10);
    memset(sd->shmaddr_pic,     '0', 10);
    memset(sd->shmaddr_keyboard,'0', 10);
    memset(sd->shmaddr_google,  '0', 10);

    memset(&sd->shmaddr_google[10], '\0', SHMSIZE-10);
    memset(&sd->shmaddr_bing[10],   '\0', SHMSIZE-10);
    memset(&sd->shmaddr_mysql[10],  '\0', SHMSIZE-10);
    memset(&sd->shmaddr_pic[10],    '\0', SHMSIZE-10);

    clearBingMysqlResultMemory ( med->bing_result, med->mysql_result );

    for ( int i=0; i<GOOGLE_SIZE; i++ )
        if ( med->google_result[i] != NULL)
            memset( med->google_result[i], '\0', SHMSIZE / GOOGLE_SIZE );
}
