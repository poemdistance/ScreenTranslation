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

char *text = NULL;
char *previousText = NULL;

volatile sig_atomic_t CanNewEntrance = 0;
volatile sig_atomic_t destroyIcon = 0;

extern char *shmaddr_google;
extern char *shmaddr_baidu;
extern char *shmaddr_selection;
extern char *shmaddr_keyboard;
extern volatile sig_atomic_t action;
extern volatile sig_atomic_t InNewWin;

extern volatile sig_atomic_t HadDestroied;

enum {

    CONTINUE,
    RETURN,
};

int waitForSelectionChangedEvent ( ) {

    struct timeval tv;
    int timeout = 300;
    double now = 0;

    printf("Check selecttion changed event\n");

    gettimeofday( &tv, NULL );
    double start =  ( tv.tv_sec*1e6 + tv.tv_usec ) / 1e3; /* ms*/

    /* 等待剪贴板变化事件，超时直接返回*/
    while ( shmaddr_selection[0] != '1' ) {
        gettimeofday( &tv, NULL );
        now = (tv.tv_sec*1e6+tv.tv_usec)/1e3;
        if ( abs ( start - now ) > timeout )  {
            if ( action == TRIBLE_CLICK ) return CONTINUE;
            pbred ( "剪贴板检测超时" );
            return RETURN;
        }
    }

    return -1;
}

void notify ( int fd[3], ConfigData *cd ) {

    FILE *fp = NULL;

    /* 禁止套娃*/
    if ( InNewWin ) {
        pbred ( "Notify: window already opened, return." );
        pbred ( "Flag: InNewWin=%d shmaddr_keyboard:%c",
                InNewWin, shmaddr_keyboard[WINDOW_OPENED_FLAG] );
        return;
    }

    char appName[100];
    int go = 0;

    switch ( waitForSelectionChangedEvent() ) {
        case RETURN: return;
        case CONTINUE: break;
        default: break;
    }

    if ( shmaddr_selection[0] == '1') {
        pbgreen ( "剪贴板已发生变化" );
        shmaddr_selection[0] = '0';
        go = 1;
    }

    if ( ! go && !(action==TRIBLE_CLICK)) {
        pred ( "剪贴板未变化, 返回" );
        CanNewEntrance = 0;
        return;
    }

    /*需每次都执行才能判断当前的窗口是什么*/
    fp = popen("ps -p `xdotool getwindowfocus getwindowpid` \
            | awk '{print $NF}' | tail -n 1", "r");

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
        text = calloc(TEXTSIZE, 1);

    if ( previousText == NULL )
        previousText = calloc(TEXTSIZE, 1);

    memset(text, 0, TEXTSIZE);

    /* FIXME:剪贴板标志位可能已经被置位，可能影响到下一次
     * 检测的正确性.*/
    if ( cd->buttonPress ) {
        pbmag ( "处理双击时检测到三击事件" );
        cd->buttonPress = 0;
        pbmag ( "Button Press: %d", cd->buttonPress );
        if ( abs(cd->pointerx-cd->previousx) > 10 
                || abs ( cd->pointery-cd->previousy ) > 10 ){
            action = SINGLE_CLICK;
            if ( !HadDestroied ) destroyIcon = 1;
            return;
        }
    }

    if ( getClipboard(text) || isEmpty(text)) {
        printf("Not copy event or empty text\n");
        previousText[0] = '\0';
        CanNewEntrance = 0;
        return ;
    }

    char *p = text;
    if ( cd->ignoreChinese && ((*p>>6)&0x03) == 3 ) {
        pbcyan ( "非Ascii码. 忽略取词. 返回" );
        return;
    }

    adjustSrcText ( text );

    if ( action == TRIBLE_CLICK ) {
        pyellow ( "text:%s<", text );
        pyellow ( "previousText:%s<", previousText );
        if ( strcmp ( text, previousText ) == 0 ) {
            pmag ( "三击获取到的文本与上次相同，关闭弹出图标" );
            if ( !HadDestroied ) destroyIcon = 1;
            return;
        }
    }

    /* 只能减小结果获取错误的概率，如果两边翻译都不够快，清零发生在百度谷歌翻译写1之前，
     * 这句是没有意义的，之后点开翻译结果界面获取到的就会有上一次点击文本的内容，不过
     * 一般按切换按钮后是可以重新加载出想要的结果的*/
    memset(shmaddr_google, '0', 10);
    memset(shmaddr_baidu, '0', 10);

    writePipe(text, fd[0]);
    writePipe(text, fd[1]);
    writePipe(text, fd[2]);

    if ( HadDestroied ) {
        CanNewEntrance = 1;
        destroyIcon = 0;
    }

    strcpy ( previousText, text );

    pbmag ( "Return from notify. Button Press: %d", cd->buttonPress );

    if ( cd->buttonPress ) {
        cd->buttonPress = 0;
        pbmag ( "Button Press 再次置零" );
        shmaddr_selection[0] = '0';
    }
}

