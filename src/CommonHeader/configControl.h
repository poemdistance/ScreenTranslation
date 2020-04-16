#ifndef __CONFIG_CONTROL__
#define __CONFIG_CONTROL__

#define SHORTCUT_CONTENT_LEN ( 64 )
#define MAX_SHORTCUT_NUM ( 120 )
#define PREF_CONTENT_LEN ( 64 )
#define MAX_PREF_NUM ( 120 )
#define BASE MAX_SHORTCUT_NUM


typedef struct ConfigData {

    int iconOffsetX;
    int iconOffsetY;
    int pointerOffsetX;
    int pointerOffsetY;
    int hideHeaderBar;
    int alwaysDisplay;
    int iconShowTime;
    int ctrlCToClose;
    int playAudioMask;
    unsigned int playAudioKeyval;
    int switchSourceMask;
    unsigned int switchSourceKeyval;
    int allowAutoAdjust;
    int disableShadowBorder;
    int shrinkShadowBorder;
    int ignoreChinese;
    int doNotMoveWindow;

}ConfigData;

int writeToConfig( char *keyName, char *value  );
char *readFromConfig( char *keyName, char *receive );
char **readFromConfigByKeyword ( char receive[][SHORTCUT_CONTENT_LEN], char *keyword );
char *readNameByKeyword ( char (*receive)[SHORTCUT_CONTENT_LEN], char *keyword );
void readNeededValueFromConfig( ConfigData *cd  );

#endif
