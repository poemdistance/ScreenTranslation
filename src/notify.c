/* 
 * 程序功能:
 *
 * 1. 检测Primary Selection变化标志位，判断是否创建翻译入口图标
 *
 * 2. 检测当前窗口是否为预设的应忽略窗口
 *
 * 3. 满足创建入口图标的情况下，获取Primary Selection内容
 *    送入Python翻译端口.
 *
 */

#include "common.h"

int fd_key = -1;
FILE *fp = NULL;
char *text = NULL;
int NoneText = 0;
int CanNewEntrance = 0;

extern char *shmaddr_google;
extern char *shmaddr_baidu;
extern char *shmaddr_selection;
extern int action;

extern int HadDestroied;

void notify(int (*history)[4], int *thirdClick, int *releaseButton, int fd[2]) {

    char appName[100];

    int pikaqiuGo = 0;

    /* 必须延迟一下, 原因:
     * 检测Primary Selection的程序跑的没这边快，
     * 需要等到对方写完1后才能继续(如果对方正在写1)*/
    usleep(100000);

    if ( shmaddr_selection[0] == '1') {

        shmaddr_selection[0] = '0';
        printf("\033[0;31mPrimary Selection changed flag: set to 0\033[0m\n");

        /* 去吧, 皮卡丘*/
        pikaqiuGo = 1;
    }

    if ( ! pikaqiuGo ) {

        action = 0;
        memset(*history, 0, sizeof(*history));
        CanNewEntrance = 0;
        return;
    }

    if ( *thirdClick == 1 )
        *thirdClick = 0;

    *releaseButton = 1;

    /* TODO: Please remove relative codes of opening this device*/
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


    if ( isApp("wantToIgnore", appName) == 1 ) {
        printf("忽略此软件\n");
        return;
    }

    if ( text == NULL )
        /*free in forDetectMouse.c*/
        if (( text = calloc(TEXTSIZE, 1)) == NULL)
            err_exit("malloc failed in notify.c");

    memset(text, 0, TEXTSIZE);
    int retval = 0;

    if ( (retval = getClipboard(text) ) == 1) {
        printf("Not copy event\n");
        action = 0;
        memset(*history, 0, sizeof(*history));
        CanNewEntrance = 0;
        return ;
    }

    /* 只能减小结果获取错误的概率，如果两边翻译都不够快，清零发生在百度谷歌翻译写1之前，
     * 这句是没有意义的，之后点开翻译结果界面获取到的就会有上一次点击文本的内容，不过
     * 一般按切换按钮后是可以重新加载出想要的结果的*/
    memset(shmaddr_google, '0', 10);
    memset(shmaddr_baidu, '0', 10);

    writePipe(text, fd[0]);
    writePipe(text, fd[1]);

    /* 情况1: 双击单词后再点击了一次形成的三击选段，此时的3击不能再弹出入口图标
     * 情况2: 从空白处直接3击取段，此时应弹出入口图标
     *
     * 总结: 只要入口图标已经创建就不应该弹出，反之反之, HadDestroied就是入口图标
     *       是否为销毁状态的标志位, 只要是销毁状态，应该弹出
     */
    if ( HadDestroied ) {

        CanNewEntrance = 1;
        printf("\033[0;35mCanNewEntrance flag set to 1 \033[0m\n");
    }
    else {

        printf("\033[0;35mCanNewEntrance flag is 0 \033[0m\n");
    }

    /*清除鼠标记录*/
    memset(*history, 0, sizeof(*history));
}

void send_Ctrl_Shift_C() {

    Display *dpy;
    unsigned int ctrl, shift, c;
    dpy = XOpenDisplay(NULL);

    ctrl = XKeysymToKeycode (dpy, XK_Control_L);
    XTestFakeKeyEvent (dpy, ctrl, True, 0);
    XFlush(dpy);

    shift = XKeysymToKeycode (dpy, XK_Shift_L);
    XTestFakeKeyEvent (dpy, shift, True, 0);
    XFlush(dpy);

    c = XKeysymToKeycode(dpy, XK_C);
    XTestFakeKeyEvent(dpy, c, True, 0);
    XFlush(dpy);

    XTestFakeKeyEvent(dpy, c, False, 0);
    XFlush(dpy);

    XTestFakeKeyEvent(dpy, shift, False, 0);
    XFlush(dpy);

    XTestFakeKeyEvent(dpy, ctrl, False, 0);
    XFlush(dpy);

    XCloseDisplay(dpy);
}

void send_Ctrl_C() {

    Display *dpy;
    unsigned int ctrl, c;
    dpy = XOpenDisplay(NULL);

    ctrl = XKeysymToKeycode (dpy, XK_Control_L);
    XTestFakeKeyEvent (dpy, ctrl, True, 0);
    XFlush(dpy);

    c = XKeysymToKeycode(dpy, XK_C);
    XTestFakeKeyEvent(dpy, c, True, 0);
    XFlush(dpy);

    XTestFakeKeyEvent(dpy, c, False, 0);
    XFlush(dpy);

    XTestFakeKeyEvent(dpy, ctrl, False, 0);
    XFlush(dpy);

    XCloseDisplay(dpy);
}

