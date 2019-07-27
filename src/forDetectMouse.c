#include "common.h" 

const static char termName[][20] =
{
    "terminator",
    "gnome-terminal-",
    "konsole"
};


const static char screenShotApp[][20] = {
    "flameshot"
};

extern char *shmaddr;
extern char *text;
extern FILE *fp;
extern int mousefd;
extern int fd_key;
extern int action;

void err_exit(char *buf) {
    fprintf(stderr, "%s\n", buf);
    exit(1);
}

void delay() {

    /*等待数据被写入剪贴板,若不延时,获取的剪贴板内容还是上次的*/
    for(int i = 0; i < 1024; i++)
        for ( int j = 0; j < 6024; j++ );

}

/*写数据到管道*/
void writePipe(char *text, int fd) {

    int writelen;

    /*排除空字符和纯回车字符*/
    if ( strcmp( text, " ") != 0 && strcmp( text, "\n") != 0 ) {

        writelen = strlen(text);
        for(int i=0; i<writelen; i++) {
            if ( text[i] == '\n')
                text[i] = ' ';
        }
        if ( text[writelen-1] != '\n')  {
            text = strcat(text, "\n");
            writelen++;
        }

        int ret = write( fd, text, writelen );
        printf("len=%d, actul writelen=%d\n", writelen, ret);
        if ( ret != writelen ) {
            fprintf(stderr, "writelen=%d,\
                    write error in forDetectMouse.c func: writePipe\n", ret);
            perror("errno");
        }
    } else {
        fprintf(stdout, "所获取为空字符串...\n");
        shmaddr[0] = EMPTYFLAG;
    }
}

/*获取子进程状态，防止僵尸进程*/
void handler(int signo) {

    pid_t pid;
    while( (pid=waitpid(-1, NULL, WNOHANG)) > 0);
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

    char *p = name;

    /*TODO:
     * NOTE: 
     * appName 需要添加回车符如果是自行赋值作为测试的,
     * 否则此循环将导致越界访问内存;
     * 
     * 呃，加个*p检测结尾字符其实也是可以的...
     * */
    while(*p++ != '\n');
    *(p-1) = '\0';

    for ( int i = 0; i < n; i++ ) {
        if ( strcmp ( app[i], name ) == 0 )
            return 1;
    }
    return -1;
}

/*获取当前数组下标的前一个下标值,
 *数组元素为4*/
int previous( int n )
{
    if ( n != 0 )
        return n - 1;
    else
        return  3;
}

/*判断当前鼠标action*/
int isAction(int history[], int last, int judgeType) {
    int m, n, j, q;
    m = previous(last);
    n = previous(m);
    j = previous(n);
    q = previous(j);

    if(judgeType == DOUBLECLICK &&
            history[m] == 0 && history[n] == 1 &&
            history[j] == 0 && history[q] == 1 ) {

        action = DOUBLECLICK;
        return 1;
    }

    else if(judgeType == SLIDE &&
            history[m] == 0 && history[n] == 1 &&
            history[j] == 1 && history[q] == 1
           ) {

        action = SLIDE;
        return 1;
    }

    return 0;
}

void quit() {

    /*退出前加个回车*/
    fprintf(stdout, "\n");

    if ( text != NULL )
        free(text);

    close(mousefd);
    close(fd_key);

    exit(0);
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
