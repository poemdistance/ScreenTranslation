#include "common.h"
#include "cleanup.h"
#include "detectMouse.h"
#include <ctype.h>

typedef struct TmpIgnore {

    char appname[30];
    struct TmpIgnore *nextapp;

}TmpIgnore;

const static char termName[][20] =
{
    "terminator",
    "gnome-terminal-",
    "konsole"
};


const static char screenShotApp[][20] = {
    "flameshot"
};

const static char wantToIgnore[][20] = {
    "VirtualBox",
    "VirtualBoxVM",
    "vlc",
    "qemu-system-arm",
    "nautilus",
    "eog",
    "gimp-2.10"
};

TmpIgnore *getLinkHead () {

    static TmpIgnore head = { { '\0' }, NULL };
    return &head;
}

int releaseLink (  ) {

    TmpIgnore *head = getLinkHead();
    TmpIgnore *p = head->nextapp;
    TmpIgnore *tmp = NULL;

    while ( p ) {

        tmp = p->nextapp;
        free ( p );
        p = tmp;
    }

    return 0;
}

int isExist(char *buf,  char *app ) {

    TmpIgnore *head = getLinkHead();
    TmpIgnore *p = head->nextapp;

    while ( p ) {
        if ( strcmp ( p->appname, app ) == 0 )
            return 1;
        p = p->nextapp;
    }

    return 0;
}

int deleteApp(char *app) {

    TmpIgnore *head = getLinkHead();
    TmpIgnore *p = head;
    TmpIgnore *tmp = NULL;

    while ( p->nextapp ) {
        if ( strcmp ( p->nextapp->appname, app ) == 0 ) {
            tmp = p->nextapp->nextapp;
            free ( p->nextapp );
            p->nextapp = tmp;
            return 1;
        }
        p = p->nextapp;
    }

    return 0;
}

TmpIgnore *appendApp( char *app ) {

    TmpIgnore *head = getLinkHead();
    TmpIgnore *p = head;

    while ( p->nextapp ) {
        p = p->nextapp;
    }

    TmpIgnore *tmp = NULL;
    tmp = calloc ( 1, sizeof(TmpIgnore) );
    strcpy ( tmp->appname, app );
    tmp->nextapp = NULL;

    p->nextapp = tmp;
    return tmp;
}

int printIgnoreApp() {

    TmpIgnore *head = getLinkHead();
    TmpIgnore *p = head->nextapp;
    while ( p ) {
        pbcyan("%s", p->appname);
        p = p->nextapp;
    }

    return 0;
}

int checkApp(char *app) {

    if ( ! app )
        return -1;

    printIgnoreApp();

    if ( isExist (NULL, app ) ) {
        pbcyan ( "Delete app" );
        deleteApp ( app );
        return 1;
    }
    else {
        pbcyan ( "Append app" );
        appendApp ( app );
        return 0;
    }
}

char *selectApp() {

    static char app[30];
    FILE *pp = popen("ps -p `xdotool selectwindow getwindowpid`|awk '{print $NF}' | tail -n 1", "r");

    if ( !pp )
        return NULL;

    if ( fread ( app, sizeof(app), 1, pp ) < 0 )
        return NULL;

    pclose ( pp );

    return app;
}

void err_exit(char *buf) {
    fprintf(stderr, "%s\n", buf);
    perror("errno");
    quit();
}

char *adjustSrcText ( char *text ) {

    char *p = text;
    char *head = NULL;
    char *p2 = NULL;

    /* 替换回车符为空格字符*/
    while ( *p ) {
        if ( *p == '\n' )
            *p = ' ';
        p++;
    }

    /* 去除前缀空白字符*/
    p = text;
    while ( *p && isspace(*p) ) p++;
    if ( !*p ) return NULL;
    head = p;

    /* 去除后缀空白字符*/
    while ( *++p );
    while ( p != head && isspace(*--p) );
    *(p+1) = '\0';

    /* 将处理后的字符串复制回text开头*/
    p = text;
    p2 = head;
    if ( p != head )
        while ( (*p++ = *p2++) );

    strcat ( text, "\n" );

    return text;
}

/*写数据到管道*/
void writePipe(char *text, int fd) {

    write( fd, text, strlen(text) );
}

/*判断当前聚焦窗口是否为终端*/
int isApp( char *appName ,char *name ) {

    int n = 0;
    const char (*app)[20] = NULL;
    if ( strcmp ( appName, "screenShotApp" ) == 0 ) {
        n = sizeof(screenShotApp) / sizeof(screenShotApp[0]);
        app = screenShotApp;
    }
    else if ( strcmp ( appName, "terminal" ) == 0 ) {
        n = sizeof(termName) / sizeof(termName[0]);
        app = termName;
    } 
    else if ( strcmp ( appName, "wantToIgnore" ) == 0) {
        n = sizeof(wantToIgnore) / sizeof(wantToIgnore[0]);
        app = wantToIgnore;
    }

    char storage[100];
    strcpy(storage, name);
    char *p = storage;

    while(*p && *p++ != '\n');
    *(p-1) = '\0';

    for ( int i = 0; i < n; i++ ) {
        if ( strcmp ( app[i], storage ) == 0 ) {
            return 1;
        }
    }
    return 0;
}

/*获取当前数组下标的前一个下标值,
 *数组元素为4*/
int previous( int n )
{
    if ( n != 0 )
        return n - 1;
    else
        return  1;
}

/*判断当前鼠标action*/
int isAction(int history[], int last, int judgeType) {

    int m, n;

    m = previous(last);
    n = previous(m);

    switch ( judgeType ) {

        case ALL_ONE:
            return history[m] == 1 && history[n] == 1 ;

        case BUTTON_PRESS:
            return  history[m] == 1 && history[n] == 0;

        case BUTTON_RELEASE:
            return history[m] == 0 && history[n] == 1;

        default:
            pred ( "Warning: Unknow judge type" );
    }

    return 0;
}


/*同步键盘*/
void sync_key(
        int *fd,
        struct input_event *event,
        int *len)
{
    event->type = EV_SYN;
    event->code = SYN_REPORT;
    event->value = 0;
    write(*fd, event, *len);
}


/*发送按键keyCode*/
void press(int fd, int keyCode)
{
    struct input_event event;

    //发送
    event.type = EV_KEY;
    event.value = 1;
    event.code = keyCode;
    gettimeofday(&event.time,0);
    write(fd,&event,sizeof(event)) ;

    //同步
    int len = (int)sizeof(event);
    sync_key(&fd, &event, &len);
}

/*释放按键*/
void release(int fd, int keyCode)
{
    struct input_event event;

    //释放
    event.type = EV_KEY;
    event.code = keyCode;
    event.value = 0;
    gettimeofday(&event.time, NULL);
    write(fd, &event, sizeof(event));

    //同步
    int len = (int)sizeof(event);
    sync_key(&fd, &event, &len);
}

/*模拟键盘操作*/
void simulateKey(int fd,  int key[], int len) {

    int i = 0;
    for(i=0; i<len; i++)
        press(fd, key[i]);

    for(i=len-1; i>=0; i--)
        release(fd, key[i]);
}

void delay() {

    /*等待数据被写入剪贴板,若不延时,获取的剪贴板内容还是上次的*/
    for(int i = 0; i < 1024; i++)
        for ( int j = 0; j < 6024; j++ );

}
