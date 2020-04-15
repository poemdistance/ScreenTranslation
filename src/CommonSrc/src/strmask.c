#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h> /* KeySym*/
#include <ctype.h>
#include <strings.h>
#include "printWithColor.h"
#include "configControl.h"
#include "useful.h"

char modifier[7][10] = {
    "Shift", "Caps", "Ctrl", "Alt", "Super", "NumLock", "ScrollLock"
};

const char upperLetterKey[][20] = {
    "Left", "Right", "Up", "Down", "End", "Next", "Prior",
    "Home", "Delete", "Pause", "BackSpace", "Enter", "Tab",
    "Return"
};

int modifier2maskTable[7] = { 0 };


int numlock_mask = 0;
int scrolllock_mask = 0;
int capslock_mask = 0;

int isInUpperLetterKeys ( char *str );

char *toStr( int mask ) {

    static char maskName[][16] = {
        "ShiftMask", "LockMask", "ControlMask", "Mod1Mask",
        "Mod2Mask", "Mod3Mask", "Mod4Mask", "Mod5Mask"
    };

    int valid = 0;
    for ( int i=0; i<8; i++ ) 
        if ( (1<<i) == mask ) {
            valid = 1; break;
        }

    if ( !valid ) {
        pred ( "Invalid mask value." );
        pred ( "Valid data: 1 2 4 8 ... 128" );
        return "";
    }

    int i = 0;
    while ( ( mask = mask / 2) ) i++;

    return maskName[i];
}


int str2mask( char *str ) {

    if ( !str ) return 0;

    int mask = 0;
    for ( int i=0; i< sizeof(modifier)/sizeof(modifier[0]); i++ )
        if ( strstr ( upperCase(str), upperCase(modifier[i]) ) ) {
            mask |= modifier2maskTable[i];
            /* pred ( "i=%d mask=%d modifier2maskTable=%d", i , mask, modifier2maskTable[i]); */
        }

    return mask;
}

char *mask2str ( int mask, char *result ) {

    result[0] = '\0';
    for ( int i=0; i<sizeof(modifier2maskTable)/sizeof(modifier2maskTable[0]); i++  ) {
        if ( mask & modifier2maskTable[i] ) {
            strcat ( result, modifier[i] );
            strcat ( result, "+" );
        }
    }

    if ( mask & GDK_SUPER_MASK ) {

        strcat ( result, "Super" );
        strcat ( result, "+" );
    }

    return result;
}

/* 跳过p之后的分隔符(<'\t'> <' '> <'+'>), 如果没有，则返回结尾字符'\0'*/
char *skipSeparator ( char *p ) {
    while ( p && *p && ( *p == '\t' || *p == ' ' || *p == '+' ) ) p++;
    return p;
}

/* 返回p之后的第一个非空白字符，如果没有，则返回结尾字符'\0'*/
char *skipBlank ( char *p ) {

    while ( p && *p && ( *p == '\t' || *p ==' ')) p++;
    return p;
}

/* 返回空白字符所在地址, 如果没有空白字符，则返回结尾字符'\0'*/
char *findBlank ( char *p ) {
    while ( p && *p && ( *p != '\t' && *p != ' ' ) ) p++;
    return p;
}

char *getRawKeyString ( char *str ) {

    if ( !str ) return NULL;

    char *p = NULL;
    char *tmp = NULL;

    p = skipBlank(str);

    /* Invalid shortcut*/
    if ( strchr ( p, '+' ) || strchr ( p, '-' ) )
        return NULL;

    tmp = p;
    p = findBlank ( p );

    if ( *p ) *p = '\0';

    /* Invalid shortcut(空格后还有内容)*/
    if ( *skipBlank(p+1) ) return NULL;

    p = tmp;
    if ( ! isInUpperLetterKeys ( p ) )
        lowerCase ( p );
    else {
        lowerCase ( p );
        *p = toupper(*p);
        if ( strcasecmp ( p, "BackSpace" ) == 0 )
            p[4] = 'S';
    }

    return strcpy ( str, p );
}

/* 判断快捷键是否有效，如果有效，返回去除modifier后的按键字符串
 * 否则返回NULL
 *
 * 如: 输入 Ctrl+Alt+v， 输出为V*/
char* getKeyString ( char *str ) {

    if ( !str ) return NULL;

    int i = 0;
    upperCase ( str );

    char *p = NULL;

    if ( ! strchr ( str, '+' ) ) {
        pred ( "Invalid shortcut: %s\n", str );
        return NULL;
    }

    p = skipBlank(str);

    /* 使p指向最后一个modifier后面的字符*/
    while ( 1 ) {

        for ( i=0; i<sizeof(modifier)/sizeof(modifier[0]); i++ ) {
            /* pgreen ( "%s", p ); */
            if ( strstr ( p,  upperCase(modifier[i]) ) == p ) {
                p = skipSeparator ( p+strlen(upperCase(modifier[i])) );
                break;
            }
        }

        /* 无剩余modifier，退出循环*/
        if ( i == sizeof(modifier)/sizeof(modifier[0]) ) break;
    }

    /* pmag ( "%s", p ); */

    /* 除去modifier还有多个键值，此快捷键无效*/
    if ( strchr ( p, '+' ) ) {
        pred ( "Invalid shortcut: %s\n", str );
        return NULL;
    }

    /* Skip blank content*/
    p = skipBlank( p );

    /* p最后指向了'\0'*/
    if ( ! *p ) {
        pred ( "Invalid shortcut: %s\n", str );
        return NULL;
    }

    /* 到这里为止，p指向一个有效字符, 保存此位置地址*/
    char *tmp = p;

    /* 跳过有效字符，寻找空白内容直到结尾字符停止*/
    p = findBlank(p);

    /* 如果上一个循环不是以'\0'终结，则可能存在空白或者其他有效字符
     * 但这是非法的，这里排除一下*/
    p = skipBlank(p);

    /* 最终p以'\0'终结，说明第有效字符后面只是空白,此快捷键配置有效*/
    if ( ! *p ) {

        /* 清除有效字符后面的空白字符(也可能不存在)*/
        p = tmp;
        p = findBlank(p);
        *p = '\0';

        /* 保持首字母大写，之后的字母需要小写*/
        lowerCase ( tmp + 1 );

        pbcyan ( "tmp:%s", tmp );

        return strcpy ( str, tmp );
    }

    /* 最终出现了非法有效字符，此快捷键配置无效*/
    return NULL;

}

int isInUpperLetterKeys ( char *str ) {

    for ( int i=0; i<sizeof(upperLetterKey)/sizeof(upperLetterKey[0]); i++ ) {
        if ( strcasecmp ( str, upperLetterKey[i] ) == 0 )
            return 1;
    }

    return 0;
}

int *extractShortcut ( Display *display ) {

    static char shortcut[MAX_SHORTCUT_NUM][SHORTCUT_CONTENT_LEN];
    memset ( shortcut, '\0', sizeof(shortcut) );

    /* 前半段用于存储key modifier对应的掩码，后半段存储对应快捷键的键值*/
    static int grabKeys[MAX_SHORTCUT_NUM*2];
    memset ( grabKeys, '\0', sizeof(grabKeys) );

    readFromConfigByKeyword ( shortcut, "Shortcut");

    printf("\n");

    for ( int i=0, j=0; i<sizeof(shortcut)/sizeof(shortcut[0]); i++ ) {

        if  ( *shortcut[i] ) {

            pyellow ( "Got shortcut config: %s", (char*)&shortcut[i] );
            grabKeys[j] = str2mask ( (char*)&shortcut[i] );

            /* Invalid shortcut config.*/
            if ( ! getKeyString((char*)&shortcut[i]) ) { 
                grabKeys[j] = 0;
                continue; 
            }

            if ( ! isInUpperLetterKeys ( (char*)&shortcut[i] ) )
                lowerCase ( (char*)&shortcut[i] );
            else {
                lowerCase ( (char*)&shortcut[i] );
                shortcut[i][0] = toupper(shortcut[i][0]);
                if ( strcasecmp ( (char*)&shortcut[i], "BackSpace" ) == 0 )
                    shortcut[i][4] = 'S';
            }

            KeySym sym = XStringToKeysym ( (char*)&shortcut[i] );
            grabKeys[BASE+j] =  XKeysymToKeycode ( display, sym);

            pmag ( "Got Mask: %d Got Key: %s\n",
                    grabKeys[j], (char*)&shortcut[i] );

            j++;
        }

        if ( ! *shortcut[i] ) break;
    }

    return grabKeys;
}


    void
getModifiersMapping (Display * dpy)
{

    static int come = 0;

    if ( come ) return;

    come = 1;

    if ( !dpy )
        dpy = XOpenDisplay ( NULL );

    int i;
    XModifierKeymap *modmap;
    KeyCode nlock, slock;

    /* 猜测:Mod1 - Mod5修饰符对应有不同的键值，比如NumLock键值可以分配
     * 到Mod1，也可以分配到Mod2，那么不同情况下，NumLock就可以对应不同
     * 的修饰符*/
    static int mask_table[8] = {
        ShiftMask, LockMask, ControlMask, Mod1Mask,
        Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
    };

    nlock = XKeysymToKeycode (dpy, XK_Num_Lock);
    slock = XKeysymToKeycode (dpy, XK_Scroll_Lock);
    KeyCode alt = XKeysymToKeycode ( dpy, XK_Alt_L );
    KeyCode capslock = XKeysymToKeycode ( dpy, XK_Caps_Lock);
    KeyCode ctrl = XKeysymToKeycode ( dpy, XK_Control_L );
    KeyCode shift = XKeysymToKeycode ( dpy, XK_Shift_L );
    KeyCode super = XKeysymToKeycode ( dpy, XK_Super_L );

    pblue ( "Super Key Keycode=%d", super );

    KeyCode keycode[7] = {
        shift, capslock, ctrl, alt, super, nlock, slock
    };

    int index = 0;

    /*
     * Find out the masks for the NumLock and ScrollLock modifiers,
     * so that we can bind the grabs for when they are enabled too.
     */
    modmap = XGetModifierMapping (dpy);

    /* max_keypermod: 此xorg服务上每个修饰符(modifier)最大含有的键值数
     * 如果为0，说明modifier被禁止了*/
    if (modmap != NULL && modmap->max_keypermod > 0)
    {
        /* 有8个修饰符，故乘8, 每个修饰符可以有max_keypermod个键值*/
        for (i = 0; i < 8 * modmap->max_keypermod; i++) 
        {
            index = i/modmap->max_keypermod;

            /* 轮询所有modifier对应的keycode，匹配当前获取到的modmap->modifiermap[i]*/
            for ( int j=0; j<sizeof(keycode)/sizeof(KeyCode); j++ ) {

                if ( keycode[j] != 0 && modmap->modifiermap[i] == keycode[j] ) {

                    pgreen ( "%s ( modifier )\t\t->\t%s maskValue=%d",
                            modifier[j], toStr(mask_table[index]), mask_table[index] );

                    modifier2maskTable[j] = mask_table[index];

                    /* 标记num lock和scroll lock是否正在使能*/
                    if ( keycode[j] == nlock ) numlock_mask = mask_table[index];
                    if ( keycode[j] == slock ) scrolllock_mask = mask_table[index];
                }
            }
        }
    }

    capslock_mask = LockMask;

    if (modmap)
        XFreeModifiermap (modmap);
}

int unusedMask() {

    static int mask = 0;
    if ( mask ) return mask;

    Display *display = NULL;
    display = XOpenDisplay ( NULL );
    if ( display == NULL ) return 0;
    getModifiersMapping(display);
    XCloseDisplay(display);

    mask = numlock_mask | capslock_mask | scrolllock_mask;

    pbblue ( "unused mask: %d numloc:%d capslock:%d scrolllock:%d", 
            mask, numlock_mask, capslock_mask, scrolllock_mask );

    return mask;
}


/* int main(int argc, char **argv) */
/* { */
/*     extractModifier(); */
/* } */
