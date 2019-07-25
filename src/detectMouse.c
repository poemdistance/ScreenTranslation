/*
 * 程序功能：
 * 1. 检测鼠标动作，捕捉双击，单击和区域选择事件，
 *    并设置action为相应的值:DOUBLECLICK,SINGLECLICK
 *    SLIDE
 *
 * 2. 模拟键盘发送CTRL-C或者CTRL-SHIFT-C进行复制操作
 *
 * 3. 通过Xserver获取剪贴板内容
 *
 * 4. Fork一个子进程，父进程通过管道将剪贴板内容送子进程
 *
 * 5. 从定向子进程标准输入为管道读取端，事exec执行的python翻译
 *    程序直接从管道中读取翻译源数据
 * */


#include "common.h"

extern char *shmaddr;

char *text = NULL;
FILE *fp = NULL;
int mousefd;
int fd_key = -1;
extern int action;


void *DetectMouse(void *arg) {

    struct sigaction sa;
    int retval ;
    char buf[3];
    char appName[100];
    int releaseButton = 1;
    fd_set readfds;
    struct timeval tv;
    struct timeval old, now;
    double oldtime = 0;
    double newtime = 0;
    double lasttime = 0;
    int thirdClick;

    int fd[2];
    int status;
    pid_t pid;

    int Ctrl_Shift_C[] = {KEY_LEFTCTRL, KEY_LEFTSHIFT, KEY_C};
    int Ctrl_C[] = {KEY_LEFTCTRL, KEY_C};

    if ( (status = pipe(fd)) != 0 ) {
        fprintf(stderr, "create pipe fail\n");
        exit(1);
    }

    if ( ( pid = fork() ) == -1 ) {
        fprintf(stderr, "fork fail\n");
        exit(1);
    }

    if ( pid > 0 ) {

        /*父进程:关闭读端口*/
        close(fd[0]);

        sa.sa_handler = handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if ( sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }

        // 打开鼠标设备
        mousefd = open("/dev/input/mice", O_RDONLY );
        if ( mousefd < 0 ) {
            fprintf(stderr, "Failed to open mice\
                    \nPlease execute with superuser");
            exit(1);
        }

        int history[4] = { 0 };
        int i = 0, n = 0, m = 0;

        /*捕捉ctrl-c退出信号*/
        signal(SIGINT, quit);


        while(1) {
            // 设置最长等待时间
            tv.tv_sec = 5;
            tv.tv_usec = 0;

            FD_ZERO( &readfds );
            FD_SET( mousefd, &readfds );
            retval = select( mousefd+1, &readfds, NULL, NULL, &tv );
            if(retval==0) {
                continue;
            }
            if(FD_ISSET(mousefd,&readfds)) {

                // 读取鼠标设备中的数据
                if(read(mousefd, buf, 3) <= 0) {
                    continue;
                }

                /*循环写入鼠标数据到数组*/
                history[i++] = buf[0] & 0x07;
                if ( i == 4 )
                    i = 0;

                /*m为最后得到的鼠标键值*/
                m = previous(i);
                n = previous(m);

                //printf("current action=%d\n", action);
                //printf("%d %d\n", history[m], history[n]);

                /*没有按下按键并活动鼠标,标志releaseButton=1*/
                if ( history[m] == 0 && history[n] == 0 ) {
                    releaseButton = 1;
                    action = 0;
                }

                /*按下左键*/
                /* 此处不要使用1 0的顺序，因为m n下标出现0 1可能是区域选择
                 * 事件(SLIDE),这将导致SLIDE被一直误判*/
                if ( history[m] == 1 && history[n] == 0 ) {
                    if ( releaseButton ) {

                        gettimeofday(&old, NULL);

                        /* lasttime为双击最后一次的按下按键时间;
                         * 如果上次双击时间到现在不超过600ms，则断定为3击事件;
                         * 3击会选中一整段，或一整句，此种情况也应该复制文本*/
                        if (abs(lasttime - ((old.tv_usec + old.tv_sec*1000000) / 1000)) < 600 \
                                && lasttime != 0 && action == DOUBLECLICK)
                            thirdClick = 1; /*3击标志*/
                        else { /*不是3击事件则按单击处理，更新oldtime*/
                            oldtime = (old.tv_usec + old.tv_sec*1000000) / 1000;
                            thirdClick = 0;
                            action = SINGLECLICK;
                        }
                        releaseButton = 0;

                        /*非3击事件，则为单击，更新oldtime后返回检测鼠标新一轮事件*/
                        if ( !thirdClick )
                            continue;
                    }
                }

                /*检测检测是否可能为双击,以及判断时间间隔(应跳过确定的3击事件)*/
                if ( isAction(history, i, DOUBLECLICK) && !thirdClick)  {
                    releaseButton = 1;
                    gettimeofday( &now, NULL );
                    newtime = (now.tv_usec + now.tv_sec*1000000) / 1000;

                    /*双击超过600ms的丢弃掉*/
                    if ( abs (newtime - oldtime) > 600)  {
                        memset(history, 0, sizeof(history));
                        continue;
                    }
                    /*更新最后一次有效双击事件的发生时间*/
                    lasttime = newtime;
                }

                /*双击,3击或者按住左键滑动区域选择事件处理*/
                if ( isAction(history, i, DOUBLECLICK)
                        || isAction(history, i, SLIDE)
                        || (thirdClick == 1)) {

                    if ( thirdClick == 1 ) {
                        thirdClick = 0;

                        /* 通知已释放左键，让检测程序能继续更新oldtime
                         * 否则下次releaseButton只能在双击事件检测里执行
                         * 造成oldtime长时未更新导致每次执行3击后的双击
                         * 都被视为超时*/
                        releaseButton = 1;
                    }

                    if ( fd_key < 0 )
                        if ((fd_key = open("/dev/input/event3", O_RDWR)) >= 0 ) 
                            printf("open event3 successful\n");


                    /*需每次都执行才能判断当前的窗口是什么*/
                    fp = popen("ps -p `xdotool getwindowfocus getwindowpid`\
                            | awk '{print $NF}' | tail -n 1", "r");

                    memset ( appName, 0, sizeof(appName) );

                    if ( fread(appName, sizeof(appName), 1, fp) < 0) {
                        fprintf(stderr, "fread error\n");
                        continue;
                    }

                    pclose(fp);

                    if ( isTerminal(appName) == 1)
                        simulateKey(fd_key, Ctrl_Shift_C, 3);
                    else
                        simulateKey(fd_key, Ctrl_C, 2);

                    delay();

                    if ( text == NULL )
                        /*free in forDetectMouse.c*/
                        text = malloc(TEXTSIZE);

                    memset(text, 0, TEXTSIZE);

                    getClipboard(text);


                    memset(shmaddr, '\0', SHMSIZE);
                    writePipe(text, fd[1]);

                    /*清除鼠标记录*/
                    memset(history, 0, sizeof(history));

                }/*双击,3击或者区域选择事件处理*/

            } /*if(FD_ISSET(mousefd,&readfds))*/

        } /*while loop*/

    } /*if pid > 0*/

    else { /*child process*/

        close(fd[1]); /*关闭写端口*/

        /*重映射标准输入为管道读端口*/
        if ( fd[0] != STDIN_FILENO) {
            if ( dup2( fd[0], STDIN_FILENO ) != STDIN_FILENO) {
                fprintf(stderr, "dup2 error");
                close(fd[0]);
                exit(1);
            }
        }

        char * const cmd[3] = {"tranen","-s", (char*)0};
        if ( execv( "./tranen", cmd ) < 0) {
            fprintf(stderr, "Execv error\n");
            exit(1);
        }
        printf("detectMouse.c 子进程已经退出...\n");
    }
    pthread_exit(NULL);
    printf("pthread_exit()...\n");
}

