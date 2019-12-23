#include "fitting.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "expanduser.h"

#define SET_DEAULT_PARAS( forWhich, a, b, c, d ) \
    \
    if ( forWhich == FOR_AUDIO_BUTTON ) {\
        *(double*)a=0.00049532,\
        *(double*)b=0.0315346,\
        *(double*)c=9.60549,\
        *(double*)d=39.2648;\
    } \
    else if ( forWhich == FOR_WIN_WIDTH ){\
        *(double*)a=0.00058368,\
        *(double*)b=0.579037,\
        *(double*)c=-17.1014,\
        *(double*)d=546.854;\
    }\
    else if ( forWhich == FOR_WIN_HEIGHT ) {\
        *(double*)a=-0.0283941,\
        *(double*)b=1.60358,\
        *(double*)c=-10.4371,\
        *(double*)d=260.154;\
    }

int notExist ( char *path ) {

    return access ( path, F_OK ) != 0;
}

/* Get the fitting function parameters stored in 'path'*/
int getFitFunc(char *path, int forWhich, double *a, double *b, double *c, double *d, int disable) {

    if ( disable == 0 ) {

        SET_DEAULT_PARAS ( forWhich, a,b,c,d );
        return 0;
    }

    FILE *fp = fopen ( path, "r" );
    if ( ! fp ) {
        printf("Open file failed (getFitFunc)\n");

        /* 未获取到参数值，使用默认值*/
        SET_DEAULT_PARAS ( forWhich, a,b,c,d );
        return 1;
    }

    fscanf(fp, "a=%lf\nb=%lf\nc=%lf\nd=%lf", a, b, c, d);
    pbgreen ( "%s", path );
    pbyellow ( "%lf %lf %lf %lf", *a, *b, *c, *d );

    /* 数据获取为空时，使用默认值*/
    if ( *a <= 0.000001 && *b <= 0.000001 && *c <= 0.000001 && *d<= 0.000001 ) {
        pbred ( "拟合函数参数读取错误(全为0)" );
        SET_DEAULT_PARAS ( forWhich, a,b,c,d );
    }

    if ( fp )
        fclose ( fp );

    return 0;
}

/* Generate the fitting function by bash command*/
void genFitFunc ( char *name, int disable ) {

    if ( disable == 0 )
        return;

    char *base = expanduser("/home/$USER/.stran/");
    char path[1024] = { '\0' };

    char cmdPart[5][512] = {  

        "gnuplot -e \"set fit quiet;set fit logfile '/home/$USER/.stran/",
        ".log';f(x)=a*x**3+b*x**2+c*x+d;fit f(x) '/home/$USER/.stran/",
        ".data' using 1:2 via a,b,c,d\" && cat /home/$USER/.stran/",
        ".log | tail -n 11 | head -n 4 | awk -F' ' '{print $1 $2 $3}' > /home/$USER/.stran/",
        ".func" 
    };

    char cmd[1024]  = { '\0' };
    strcat ( strcat ( strcat ( path, base ), name), ".data");

    if ( notExist ( path )) {
        pbred ( "%s not exist", path );
        fclose ( fopen ( path, "w" ) );
    }

    for ( int i=0; i<4; i++ )
        strcat ( cmdPart[i], name );

    for ( int i=0; i<5; i++ )
        strcat ( cmd, cmdPart[i] );

    system ( cmd );
}
