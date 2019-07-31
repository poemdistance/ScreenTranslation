/*程序功能:
 *
 * 1. 模拟键盘发送CTRL-C或者CTRL-SHIFT-C进行复制操作
 *
 * 2. 通过Xserver获取剪贴板内容
 *
 * 3. 对相关标志变量重设
 *
 * 4. 清除鼠标历史信息
 *
 * */

#include "common.h"

int fd_key = -1;
FILE *fp = NULL;
char *text = NULL;
int NoneText = 0;
char *lastText = NULL;

extern int CanCopy;
extern int CanNewEntry;
extern char *shmaddr;
extern int action;

void notify(int (*history)[4], int *thirdClick, int *releaseButton, int fd[2]) {

    int Ctrl_Shift_C[] = {KEY_LEFTCTRL, KEY_LEFTSHIFT, KEY_C};
    int Ctrl_C[] = {KEY_LEFTCTRL, KEY_C};
    char appName[100];

    if ( lastText == NULL )  {
        lastText = calloc( TEXTSIZE , 1);
        if ( lastText == NULL ) 
            err_exit("malloc for lastText failed in notify.c");
    }

    if ( *thirdClick == 1 )
        *thirdClick = 0;
    *releaseButton = 1;

    if ( fd_key < 0 )
        if ((fd_key = open("/dev/input/event3", O_RDWR)) < 0 ) 
            err_exit("opened keyboard device fail");


    /*需每次都执行才能判断当前的窗口是什么*/
    fp = popen("ps -p `xdotool getwindowfocus getwindowpid`\
            | awk '{print $NF}' | tail -n 1", "r");

    memset ( appName, 0, sizeof(appName) );

    if ( fread(appName, sizeof(appName), 1, fp) < 0) {
        fprintf(stderr, "fread error\n");
        return;
    }

    pclose(fp);

    fprintf(stdout, "Focus window application: %s\n", appName);

    if ( isApp("screenShotApp", appName) == 1)
        return;

    if ( isApp("terminal", appName) == 1) {
        printf("send key ctrl-shift-c\n");
        simulateKey(fd_key, Ctrl_Shift_C, 3);
        printf("Send key successful\n");
    }
    else {
        printf("send key ctrl-c\n");
        simulateKey(fd_key, Ctrl_C, 2);
        printf("Send key successful\n");
    }
    delay();

    if ( text == NULL )
        /*free in forDetectMouse.c*/
        if (( text = calloc(TEXTSIZE, 1)) == NULL)
            err_exit("malloc failed in notify.c");

    memset(text, 0, TEXTSIZE);
    getClipboard(text);

    printf("-----------------%s----------------\n", text);

    CanCopy = 1;
    if ( strcmp(lastText, text ) == 0 )
    {
        *text = '0';
        action = 0;
        static int i = 0;
        printf("same text %d %d %d %d %d\n", i++, (*history)[0], (*history)[1],(*history)[2],(*history)[3]);
        memset(*history, 0, sizeof(*history));
        CanCopy = 0;
        CanNewEntry = 0;
        return ;
    }

    strcpy(lastText, text);

    memset(shmaddr, '\0', SHMSIZE);
    writePipe(text, fd[1]);

    /*管道写完成，可以创建入口图标了
     * 但是对于判断action==DOUBLECLICK进去的不用改写该标志变量
     * 防止多次显示入口图标*/
    if ( *thirdClick || action == SLIDE)
        CanNewEntry = 1;
    CanCopy = 0;

    /*清除鼠标记录*/
    memset(*history, 0, sizeof(*history));
}
