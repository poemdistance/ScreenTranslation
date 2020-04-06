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

#include <poll.h>
#include "common.h"
#include "detectMouse.h"
#include "cleanup.h"
#include "printWithColor.h"

FILE *fp = NULL;
char *text = NULL;

volatile sig_atomic_t CanNewEntrance = 0;
volatile sig_atomic_t destroyIcon = 0;

extern char *shmaddr_google;
extern char *shmaddr_baidu;
extern char *shmaddr_selection;
extern char *shmaddr_keyboard;
extern volatile sig_atomic_t action;
extern volatile sig_atomic_t InNewWin;

extern volatile sig_atomic_t HadDestroied;

void notify ( int fd[3], ConfigData *cd ) {

    /* 禁止套娃*/
    if ( InNewWin ) {
        pbred ( "Notify: window already opened, return." );
        pbred ( "Flag: InNewWin=%d shmaddr_keyboard:%c",
                InNewWin, shmaddr_keyboard[WINDOW_OPENED_FLAG] );
        return;
    }

    char appName[100];
    struct timeval tv;
    double now = 0;
    int go = 0;
    int count = 0;
    int timeout = 300;

    gettimeofday( &tv, NULL );
    double start =  ( tv.tv_sec*1e6 + tv.tv_usec ) / 1e3; /* ms*/

    printf("Check selecttion changed event %d\n", count++);

    /* 战略性休眠*/
    usleep ( 150*1e3 );

    /* 等待剪贴板变化事件，超时直接返回*/
    while ( shmaddr_selection[0] != '1' ) {

        gettimeofday( &tv, NULL );
        now = (tv.tv_sec*1e6+tv.tv_usec)/1e3;
        if ( action == DOUBLE_CLICK && cd->buttonPress ) {
            pmag ( "Trible click in notify" );
            timeout += 300;
        }
        if ( abs ( start - now ) > timeout )  {
            pbred ( "剪贴板检测超时返回" );
            if ( action == TRIBLE_CLICK ) destroyIcon = 1;
            return;
        }
    }

    if ( shmaddr_selection[0] == '1') {
        pbgreen ( "剪贴板已发生变化" );
        shmaddr_selection[0] = '0';
        go = 1;
    }

    if ( ! go ) {
        pred ( "剪贴板未变化, 返回" );
        CanNewEntrance = 0;
        return;
    }

    /*需每次都执行才能判断当前的窗口是什么*/
    fp = popen("ps -p `xdotool getwindowfocus getwindowpid` | awk '{print $NF}' | tail -n 1", "r");

    memset ( appName, 0, sizeof(appName) );

    if ( fread(appName, sizeof(appName), 1, fp) < 0) {
        fprintf(stderr, "fread error\n");
        return;
    }

    pclose(fp);

    fprintf(stdout, "Focus window application: %s", appName);


    if ( isApp("wantToIgnore", appName)) {
        printf("此软件在忽略名单\n");
        return;
    }
    else if ( isExist ( "TmpIgnore", appName ) ) {
        pbred ( "暂时忽略此软件，<Alt-V> 选中此软件解锁" );
        return;
    }

    if ( text == NULL )
        /*free in forDetectMouse.c*/
        if (( text = calloc(TEXTSIZE, 1)) == NULL)
            err_exit("malloc failed in notify.c");

    memset(text, 0, TEXTSIZE);
    int retval = 0;

    /* TODO:返回值已经没有1*/
    if ( (retval = getClipboard(text) ) == 1 || isEmpty(text)) {
        printf("Not copy event or empty text\n");
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
    writePipe(text, fd[2]);

    /* 情况1: 双击单词后再点击了一次形成的三击选段，此时的3击不能再弹出入口图标
     * 情况2: 从空白处直接3击取段，此时应弹出入口图标
     *
     * 总结: 只要入口图标已经创建就不应该弹出，反之反之, HadDestroied就是入口图标
     *       是否为销毁状态的标志位, 只要是销毁状态，应该弹出
     */

    if ( HadDestroied ) {
        CanNewEntrance = 1;
        destroyIcon = 0;
    }

    pbgreen ( "Return from notify" );
}

