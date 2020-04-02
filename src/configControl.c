#include <unistd.h>
#include <gdk/gdk.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "printWithColor.h"
#include "expanduser.h"
#include "configControl.h"
#include "useful.h"
#include "shortcutListener.h"

typedef struct Link {

    char data[512];
    struct Link *next;

}Link;

static char configFile[] = "/home/$USER/.stran/.configrc";

int writeToConfig( char *keyName, char *value  ) {

    /* No key, no value, just return*/
    if ( !strlen ( keyName ) || !strlen ( value ) )
        return 0;

    Link head = { '\0' };
    Link *p = NULL, *tmp = NULL;
    Link *tail = &head;

    char readBuf[512] = { '\0' };
    FILE *fp = NULL;
    char line[512] = { '\0' };
    int found = 0;

    fp = fopen(expanduser(configFile), "a+");
    if ( fp == NULL )
        return 0;

    strcat ( line, keyName );
    strcat ( line, ": " );
    strcat ( line, value );
    strcat ( line, "\n" );

    /* Read all lines into Link*/
    while ( fgets ( readBuf, sizeof(readBuf), fp ) ) {

        p = calloc ( 1, sizeof(Link) );
        strcpy ( p->data, readBuf );
        p->next = NULL;
        tail->next =  p;
        tail = p;

        /* Replace the line which contain the keyName*/
        if  ( strstr ( readBuf, keyName ) ) {
            found = 1;
            strcpy ( p->data, line );
        }
    }

    /* Not fond the keyName. Just appending it into Link*/
    if ( !found ) {
        p = calloc ( 1, sizeof(Link) );
        strcpy ( p->data, line );
        p->next = NULL;
        tail->next =  p;
        tail = p;
    }

    p = head.next;

    /* Clear all content of file*/
    fp = freopen ( expanduser(configFile), "w", fp ); 

    /* Write back the data into file and free the Link*/
    while ( p ) {
        fputs ( p->data, fp );
        tmp = p;
        p = p->next;
        free ( tmp );
    }

    fclose ( fp );

    return 1;
}

char *readFromConfig( char *keyName, char *receive ) {

    FILE *fp = NULL;
    char *p = NULL;
    char *tmp = NULL;

    fp = fopen(expanduser(configFile), "a+");
    if ( fp == NULL )
        return NULL;

    char readBuf[512];
    while ( fgets ( readBuf, sizeof(readBuf), fp ) ) {

        if ( strstr ( readBuf, keyName ) ) {
            p = strchr ( readBuf, ':' );
            /* 防止p为空，防止超出字符串结尾界限，在此基础上跳过所有'\t'和' '*/
            while ( p && *p && (*++p == '\t' ||  *p == ' ') );

            if ( !p ) break;

            tmp = p;
            while ( *p ) {
                if ( *p == '\n' )
                    *p = '\0';
                p++;
            }

            p = tmp;
            lowerCase ( p );
            *p = toupper(*p);
            while ( ( p = strchr ( p, '-' ) ) ) {
                p++;
                *p = toupper(*p);
            }

            fclose ( fp );
            pbcyan ( "return buf: %s", tmp );
            return strcpy ( receive, tmp);
        }
    }

    fclose ( fp );
    return NULL;
}

char **readFromConfigByKeyword ( char receive[][SHORTCUT_CONTENT_LEN], char *keyword ) {

    if ( ! receive )
        return (char**)receive;

    char configFile[] = "/home/$USER/.stran/.configrc";
    FILE *fp = NULL;
    char *p = NULL;

    fp = fopen(expanduser(configFile), "a+");
    if ( fp == NULL )
        return NULL;

    int i=0;
    char readBuf[512];
    char partKeyName[32];
    strcpy ( partKeyName, keyword );
    while ( fgets ( readBuf, sizeof(readBuf), fp ) ) {

        if ( strstr ( upperCase(readBuf), upperCase(partKeyName) ) ) {
            p = strchr ( readBuf, ':' );
            /* 防止p为空，防止超出字符串结尾界限，在此基础上跳过所有'\t'和' '*/
            while ( p && *p && (*++p == '\t' ||  *p == ' ') );
            if ( p ) strcpy ( (char*)&receive[i++], p );

            /* Replace '\n' with '\0' */
            if ( ( p = (char*)&receive[i-1] )) {
                while ( *p++ );
                if ( *(p=p-2) == '\n'  )
                    *p = '\0';
            }

            if ( i == MAX_SHORTCUT_NUM ) {
                pred ( "快捷键存储数组容量不足,终止搜索剩余快捷键" );
                break;
            }
        }
    }

    fclose ( fp );
    return (char**)receive;
}

char *readNameByKeyword ( char (*receive)[SHORTCUT_CONTENT_LEN], char *keyword ) {

    if ( ! receive )
        return (char*)receive;

    FILE *fp = NULL;
    char *p = NULL;

    fp = fopen(expanduser(configFile), "a+");
    if ( fp == NULL )
        return NULL;

    int i=0;
    char readBuf[512];
    char partKeyName[32];
    strcpy ( partKeyName, keyword );
    char *p2 = NULL;
    while ( fgets ( readBuf, sizeof(readBuf), fp ) ) {

        if ( (p = strstr ( upperCase(readBuf), upperCase(partKeyName) ) ) )  {

            *(p-1) = '\0';

            /* 先将首字母后所有字母转换为小写*/
            p =  readBuf + 1;
            lowerCase ( p );

            /* 将所有'-'符号后的字符大写化*/
            p = readBuf;
            while ( (p2 = strchr ( p, '-' ) ) ) {
                p2 = p2 + 1;
                *p2 = toupper ( *p2 );
                p = p2 + 1;
            }

            p2 = strchr ( readBuf, '+' );
            if ( p2 ) *(p2+1) = toupper(*(p2+1));

            strcpy ( receive[i++], readBuf );

            if ( i == MAX_SHORTCUT_NUM ) {
                pred ( "快捷键存储数组容量不足,终止搜索剩余快捷键" );
                break;
            }
        }
    }

    fclose ( fp );
    return (char*)receive;
}

void readNeededValueFromConfig( ConfigData *cd  ) {

    char buf[64] = { '\0' };

    cd->iconOffsetX =
        str2int(readFromConfig ( "Icon-Popup-Offset-X", buf ) );
    cd->iconOffsetY =
        str2int(readFromConfig ( "Icon-Popup-Offset-Y", buf ) );
    cd->pointerOffsetX = 
        str2int(readFromConfig ("Pointer-Offset-X", buf) );
    cd->pointerOffsetY = 
        str2int(readFromConfig ("Pointer-Offset-Y", buf) );
    cd->hideHeaderBar = 
        str2bool(readFromConfig ( "Hide-Header-Bar-Pref", buf ));
    cd->alwaysDisplay = 
        str2bool(readFromConfig ( "Click-Outside-To-Close-Window-Pref", buf ));
    cd->iconShowTime = 
        str2int ( readFromConfig ( "Icon-Show-Time" , buf) );
    cd->ctrlCToClose = 
        str2bool ( readFromConfig ( "Control+C-To-Close-Window-Pref" , buf) );

    cd->switchSourceMask = 
        str2mask ( readFromConfig ( "Switch-Translation-Source-Shortcut", buf ) );

    if ( ! getKeyString ( buf ) ) getRawKeyString ( buf );
    cd->switchSourceKeyval = gdk_keyval_from_name ( (buf) );


    cd->playAudioMask = 
        str2mask ( readFromConfig ( "Play-Audio-Shortcut", buf ) );

    if ( ! getKeyString ( buf ) ) getRawKeyString ( buf );
    cd->playAudioKeyval = gdk_keyval_from_name ( (buf) );

    cd->allowAutoAdjust = 
        str2bool ( readFromConfig ( "Allow-Auto-Adjust-Popup-Window-Pref" , buf ) );

    cd->disableShadowBorder = 
        str2bool ( readFromConfig ( "Disable-Shadow-Border-Pref", buf ) );

    printf("iconOffsetX:%d\n", cd->iconOffsetX);
    printf("iconOffsetY:%d\n", cd->iconOffsetY);
    printf("pointerOffsetX:%d\n", cd->pointerOffsetX);
    printf("pointerOffsetY:%d\n", cd->pointerOffsetY);
    printf("hideHeaderBar:%d\n", cd->hideHeaderBar);
    printf("alwaysDisplay:%d\n", cd->alwaysDisplay);
    printf("iconShowTime:%d\n", cd->iconShowTime);
    printf("ctrlToClose:%d\n", cd->ctrlCToClose);
    printf("switchSourceMask:%d %d\n", cd->switchSourceMask, cd->switchSourceKeyval);
    printf("playAudioMask:%d keyval:%d\n", cd->playAudioMask, cd->playAudioKeyval);
    printf("allowAutoAdjust:%d\n", cd->allowAutoAdjust);
    printf("disableShadowBorder:%d\n", cd->disableShadowBorder);
}

/* int main(int argc, char **argv) */
/* { */
/*     char readBuf[512] = { '\0' }; */
/*     if ( readFromConfig("Test2", readBuf) ) */
/*         printf("%d", atoi(readBuf)); */
/* } */
