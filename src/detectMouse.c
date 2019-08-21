/*
 * 程序功能：
 * 1. 检测鼠标动作，捕捉双击，单击和区域选择事件，
 *    并设置action为相应的值:DOUBLECLICK,SINGLECLICK, SLIDE
 *
 * 2. Fork一个子进程，父进程通过管道将剪贴板内容送入子进程
 *
 * 3. 重定向子进程标准输入为管道读取端，使exec执行的python翻译
 *    程序直接从管道中读取翻译源数据
 * */


#include "common.h"

extern char *shmaddr;
extern int InNewWin;

int mousefd;

extern int action;


void *DetectMouse(void *arg) {

    struct sigaction sa;
    int retval ;
    char buf[3];
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
                    \nTry to execute as superuser or add \
                    current user to group which /dev/input/mice belong to\n");
            exit(1);
        }

        int history[4] = { 0 };
        int i = 0, n = 0, m = 0/*, j=0, x=0*/;

        signal(SIGINT, quit);

        while(1) {

            /*超时时间*/
            tv.tv_sec = 0;
            tv.tv_usec = 500000;

            FD_ZERO( &readfds );
            FD_SET( mousefd, &readfds );

            retval = select( mousefd+1, &readfds, NULL, NULL, &tv );

            /*超时*/
            if(retval==0) {
                continue;
            }

            if(read(mousefd, buf, 3) <= 0) {
                continue;
            }

            /*打开翻译窗口后不再判断鼠标动作
             * NOTE: 这句要放在读鼠标设备的语句之后,防止窗口关闭后旧数据
             * 被读进history误判，而把任何时候的数据都读完了,
             * 其他代码逻辑才不会被旧数据影响*/
            if ( InNewWin == 1 )
                continue;

            /*循环写入鼠标数据到数组*/
            history[i++] = buf[0] & 0x07;
            if ( i == 4 )
                i = 0;

            /*m为最后得到的鼠标键值*/
            m = previous(i);
            n = previous(m);
            //j = previous(n);
            //x = previous(j);

            //printf("%d %d %d %d\n", history[m], history[n], history[j], history[x]);

            /*没有按下按键并活动鼠标,标志releaseButton=1*/
            if ( history[m] == 0 && history[n] == 0 ) {
                releaseButton = 1;
                action = 0;
            }

            /*按下左键*/
            /* 此处不要改变1 0的顺序，因为m n下标出现0 1可能是区域选择
             * 事件(SLIDE),这将导致SLIDE被一直误判*/
            if ( history[m] == 1 && history[n] == 0 ) {
                if ( releaseButton ) {

                    gettimeofday(&old, NULL);

                    /* lasttime为双击最后一次的按下按键时间;
                     * 如果上次双击时间到现在不超过600ms，则断定为3击事件;
                     * 3击会选中一整段，或一整句，此种情况也应该复制文本*/
                    if (abs(lasttime - ((old.tv_usec + old.tv_sec*1000000) / 1000)) < 800 \
                            && lasttime != 0 && action == DOUBLECLICK) {

                        thirdClick = 1;
                        notify(&history, &thirdClick, &releaseButton, fd);
                    }
                    else { /*不是3击事件则按单击处理，更新oldtime*/
                        oldtime = (old.tv_usec + old.tv_sec*1000000) / 1000;
                        thirdClick = 0;
                        action = SINGLECLICK;
                    }
                    releaseButton = 0;

                    /*非3击事件，则为单击，更新oldtime后返回检测鼠标新一轮事件*/
                    if ( !thirdClick)
                        continue;
                }
            }

            /*检测检测是否可能为双击,以及判断时间间隔(应跳过确定的3击事件)*/
            if ( isAction(history, i, DOUBLECLICK) && !thirdClick )  {
                releaseButton = 1;
                gettimeofday( &now, NULL );
                newtime = (now.tv_usec + now.tv_sec*1000000) / 1000;

                /*双击超过700ms的丢弃掉*/
                if ( abs (newtime - oldtime) > 700)  {
                    memset(history, 0, sizeof(history));
                    continue;
                }
                /*更新最后一次有效双击事件的发生时间*/
                lasttime = newtime;

                notify(&history, &thirdClick, &releaseButton, fd);
                continue;
            }

            if ( isAction( history, i, SLIDE ) )
                notify(&history, &thirdClick, &releaseButton, fd);

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
        if ( execv( "/usr/bin/tranen", cmd ) < 0) {
            fprintf(stderr, "Execv error\n");
            exit(1);
        }
        printf("detectMouse.c 子进程已经退出...\n");
        exit(1);
    }
    pthread_exit(NULL);
    printf("pthread_exit()...\n");
}

