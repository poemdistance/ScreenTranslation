/*
 * 程序功能：
 * 1. 检测鼠标动作，捕捉双击，单击和区域选择事件，
 *    并设置md->action为相应的值:DOUBLE_CLICK,SINGLE_CLICK, SLIDE
 *
 * 2. Fork一个子进程，父进程通过管道将剪贴板内容送入子进程
 *
 * 3. 重定向子进程标准输入为管道读取端，使exec执行的python翻译
 *    程序直接从管道中读取翻译源数据
 *
 * 4. 第3个子进程用于检测Primary Selection变化，以排除窗口拖动
 *    和重设窗口大小值事件, 防止翻译入口图标误弹
 * */


#include "common.h"
#include "quickSearch.h"
#include "detectMouse.h"
#include "cleanup.h"
#include "configControl.h"
#include "windowData.h"
#include <bsd/unistd.h>

#define DOUBLE_CLICK_TIMEOUT  ( 450 )

int checkOtherProcessNotifyEvent ( int fd_python[], Arg *arg ) {

    CommunicationData *md = arg->md;
    ShmData *sd = arg->sd;
    MemoryData *med = arg->med;

    if ( sd->shmaddr_searchWin[TEXT_SUBMIT_FLAG] == '1') {

        pbcyan ( "Quick Search 完毕" );

        if ( med->text == NULL )
            if (( med->text = calloc(TEXTSIZE, 1)) == NULL)
                err_exit("calloc failed in notify.c");

        sd->shmaddr_searchWin[TEXT_SUBMIT_FLAG] = '0';
        strcpy ( med->text,  &sd->shmaddr_searchWin[SUBMIT_TEXT] );
        adjustSrcText ( med->text );
        writePipe ( &sd->shmaddr_searchWin[SUBMIT_TEXT], fd_python[0] );
        writePipe ( &sd->shmaddr_searchWin[SUBMIT_TEXT], fd_python[1] );
        writePipe ( &sd->shmaddr_searchWin[SUBMIT_TEXT], fd_python[2] );
        md->canNewWin = 1;
    }

    if ( sd->shmaddr_pic[0] == FINFLAG ) {

        pbcyan ( "Tran pic 完毕" );

        if ( med->text == NULL )
            if (( med->text = calloc(TEXTSIZE, 1)) == NULL)
                err_exit("calloc failed in notify.c");

        sd->shmaddr_pic[0] = CLEAR;
        strcpy ( med->text,  &sd->shmaddr_pic[ACTUALSTART] );
        adjustSrcText ( med->text );
        writePipe ( &sd->shmaddr_pic[ACTUALSTART], fd_python[0] );
        writePipe ( &sd->shmaddr_pic[ACTUALSTART], fd_python[1] );
        writePipe ( &sd->shmaddr_pic[ACTUALSTART], fd_python[2] );
        md->canNewEntrance = 1;
    }

    if ( sd->shmaddr_keyboard[RECALL_PREVIOUS_TRAN] == '1' ) {

        pbcyan ( "打开上一次翻译内容" );
        sd->shmaddr_keyboard[RECALL_PREVIOUS_TRAN] = '0';
        md->recallPreviousFlag = 1;
        if ( med->text == NULL )
            if (( med->text = calloc(TEXTSIZE, 1)) == NULL)
                err_exit("calloc failed in notify.c");

        pbcyan ( "Previous med->text: %s", med->text );
        if ( strlen ( med->text ) ) {
            adjustSrcText ( med->text );
            writePipe ( med->text, fd_python[0] );
            writePipe ( med->text, fd_python[1] );
            writePipe ( med->text, fd_python[2] );
            pbmag ( "内容送入翻译端" );
            md->canNewWin = 1;
        }
        else { pbred ( "上一次翻译内容为空，此次不调出窗口" );}
    }

    if ( sd->shmaddr_keyboard[SELECT_EXCLUDE_FLAG] == '1' ) {
        checkApp ( selectApp() );
        sd->shmaddr_keyboard[SELECT_EXCLUDE_FLAG] = '0';
    }

    return 0;
}

void *DetectMouse(void *arg) {

    pbblue ( "启动线程DetectMouse" );
    /* ConfigData *cd = ((Arg*)arg)->cd; */
    CommunicationData *md = ((Arg*)arg)->md;
    ShmData *sd = ((Arg*)arg)->sd;
    ShmIdData *sid = ((Arg*)arg)->sid;

    setpgid ( getpid(), getppid() );

    struct timeval timer;
    int startTimer = 0;
    int checkTimeout = 1;
    int slideCount = 1;
    double start = 0;
    double now = 0;
    int buttonPress = 0;

    int fd_google[2];
    int fd_baidu[2];
    int fd_mysql[2];
    int fd_python[3];
    int status;
    pid_t pid_google = -1;

    if ( (status = pipe(fd_google)) != 0 ) {
        fprintf(stderr, "create pipe fail (google)\n");
        exit(1);
    }

    if ( (status = pipe(fd_baidu)) != 0 ) {
        fprintf(stderr, "create pipe fail (baidu)\n");
        exit(1);
    }

    if ( (status = pipe(fd_mysql)) != 0 ) {
        fprintf(stderr, "create pipe fail (mysql)\n");
        exit(1);
    }

    if ( ( pid_google = fork() ) == -1 ) {
        fprintf(stderr, "fork fail\n");
        exit(1);
    }

    /*只能放在主进程中执行*/
    if ( pid_google > 0 ) {

        if ( ( sid->pid_bing = fork() ) == -1 ) {
            fprintf(stderr, "fork fail\n");
            exit(1);
        }

        /*让子进程中pid_google=-1,防止子进程执行到pid_google>0的代码段*/
        if (sid->pid_bing == 0)
            pid_google = -1;
    }

    /* fork一个子进程用于检测剪贴板变化*/
    if ( pid_google > 0 ) {

        if ( ( sid->pid_selection = fork() ) == -1) 
            err_exit ("Fork check_selectionEvent failed");

        if ( sid->pid_selection == 0) {

            pid_google = -1;
            setproctitle ( "%s", "Check Selection Changed" );
            checkSelectionChanged( arg );
        }
    }

    /* 再fork一个用于离线翻译*/
    if ( pid_google > 0 ) {

        if ( ( sid->pid_mysql = fork() ) == -1) 
            err_exit ("Fork check_selectionEvent failed");

        /* In child process*/
        if ( sid->pid_mysql == 0 ) {

            pid_google = -1;
            sid->pid_mysql = 0;
        }
    }

    /* fork一个子进程用于检测截图识别*/
    if ( pid_google > 0 ) {

        if ( ( sid->pid_tranpic = fork() ) == -1) 
            err_exit ("Fork detectTranPicAction() failed");

        /* In child process*/
        if ( sid->pid_tranpic == 0 ) {

            pid_google = -1;

            setproctitle ( "%s", "Check Tran Pic Action" );

            /* 启动截图识别检测进程*/
            detectTranPicAction();
        }
    }

    if ( pid_google > 0 ) {

        pbmag ( "DetectMouse main process: %d", getpid() );

        /*父进程:关闭读端口*/
        close(fd_google[0]);
        close(fd_baidu[0]);

        fd_python[0] = fd_google[1];
        fd_python[1] = fd_baidu[1];
        fd_python[2] = fd_mysql[1];

        while(1) {

            usleep ( 1000 );

            if ( md->sigtermNotify ) {
                pbred ( "Detect Mouse 接收到退出信号 !!!" );
                break;
            }

            checkOtherProcessNotifyEvent( fd_python, arg );

            if ( checkTimeout ) {
                gettimeofday ( &timer, NULL );
                now = (timer.tv_usec + timer.tv_sec*1e6) / 1e3;
                if ( abs ( now-start ) > DOUBLE_CLICK_TIMEOUT ) {
                    pred ( "超时 md->action asign value: NO_ACTION" );
                    if ( !md->startSlide ) md->action = NO_ACTION;
                    checkTimeout = 0;
                }
            }

            if ( md->buttonPress ) {
                pgreen ( "................Button Press..............." );
                md->buttonState = BUTTON_PRESS;
                md->buttonPress = 0;
                buttonPress = 1;
            }

            static int printLock = 1;
            if ( md->startSlide ) {
                if ( printLock ) {
                    printf("Action to start slide\n");
                    printLock = 0;
                }
                slideCount++;
                md->action = START_SLIDE;
                buttonPress = 0;
            }

            if ( md->buttonRelease ) {
                md->buttonState = BUTTON_RELEASE;
                md->buttonRelease = 0;
                if ( md->action != DOUBLE_CLICK )
                    buttonPress = 0;
                if ( md->action == START_SLIDE ) {
                    buttonPress = 1;
                    md->startSlide = 0;
                    printLock = 1;
                } 
            }


            if ( buttonPress )
            {
                startTimer = 1;
                buttonPress = 0;

                pbyellow ( "Switch Action" );

                switch ( md->action ) {
                    case TRIBLE_CLICK:
                    case SLIDE:
                    case NO_ACTION: 
                        pbmag("~~~~~~~~~~~~~~~~Single click~~~~~~~~~~~~~~~~");
                        md->action = SINGLE_CLICK;
                        sd->shmaddr_selection[0] = '0';
                        if ( slideCount > 16 || slideCount == 1 ) md->destroyIcon = 1;
                        else {
                            pbyellow("^^^^^^ 忽略两次单击中的轻微滑动，视为双击^^^^^^");
                            md->action = DOUBLE_CLICK;
                            notify ( fd_python, arg );
                        }
                        slideCount = 1;
                        break;
                    case SINGLE_CLICK:
                        pbmag("~~~~~~~~~~~~~~~~Double click~~~~~~~~~~~~~~~~");
                        md->action = DOUBLE_CLICK;
                        md->previousx = md->pointerx;
                        md->previousy = md->pointery;
                        notify ( fd_python, arg );
                        break;
                    case DOUBLE_CLICK:
                        pbmag("~~~~~~~~~~~~~~~~Trible click~~~~~~~~~~~~~~~~");
                        if ( abs(md->pointerx-md->previousx) > 10 
                                || abs ( md->pointery-md->previousy ) > 10 ){
                            md->action = SINGLE_CLICK;
                            md->destroyIcon = 1;
                            break;
                        }
                        md->action = TRIBLE_CLICK;
                        notify ( fd_python, arg );
                        break;
                    case START_SLIDE:
                        md->action = SLIDE;
                        pbmag ( ">>>>>>>>>>>>>>>>> SLIDE <<<<<<<<<<<<<<<<<<<<<<" );
                        notify ( fd_python, arg );
                        break;

                    default:
                        pred ( "Unknow md->action" );
                        break;
                }
            }

            if ( startTimer ) {
                gettimeofday ( &timer, NULL );
                start = (timer.tv_usec + timer.tv_sec*1e6) / 1e3;
                checkTimeout = 1;
                startTimer = 0;
            }

        } /*while loop*/

    } /*if pid_google > 0*/

    /*child process for google translate*/
    if (pid_google == 0){ 

        pbgreen("执行谷歌翻译程序");

        close(fd_google[1]); /*关闭写端口*/

        /*重映射标准输入为管道读端口*/
        if ( fd_google[0] != STDIN_FILENO) {
            if ( dup2( fd_google[0], STDIN_FILENO ) != STDIN_FILENO) {
                fprintf(stderr, "dup2 error (google)");
                close(fd_google[0]);
                exit(1);
            }
        }

        char * const cmd[3] = {"tranen","-s", (char*)0};
        if ( execv( "/usr/bin/tranen", cmd ) < 0) {
            fprintf(stderr, "Execv error (google)\n");
            perror("Execv error(google):");
            exit(1);
        }
        printf("detectMouse.c (google)子进程已经退出...\n");
        exit(1);
    }

    if ( sid->pid_bing == 0 ) {

        close(fd_baidu[1]); /*关闭写端口*/

        /*重映射标准输入为管道读端口*/
        if ( fd_baidu[0] != STDIN_FILENO) {
            if ( dup2( fd_baidu[0], STDIN_FILENO ) != STDIN_FILENO) {
                fprintf(stderr, "dup2 error (baidu)");
                close(fd_baidu[0]);
                exit(1);
            }
        }

        pbgreen("执行百度翻译程序");
        char * const cmd[3] = {"bing","-s", (char*)0};
        if ( execv( "/usr/bin/bing", cmd ) < 0) {
            fprintf(stderr, "Execv error (baidu)\n");
            perror("Execv error(baidu):");
            exit(1);
        }
        pbgreen("detectMouse.c (baidu)子进程已经退出...\n");
        exit(1);

    }

    if ( sid->pid_mysql == 0 ) {

        close(fd_mysql[1]); /*关闭写端口*/

        /*重映射标准输入为管道读端口*/
        if ( fd_mysql[0] != STDIN_FILENO) {
            if ( dup2( fd_mysql[0], STDIN_FILENO ) != STDIN_FILENO) {
                fprintf(stderr, "dup2 error (mysql)");
                close(fd_mysql[0]);
                exit(1);
            }
        }

        pbgreen("Execute the offline translation process\n");
        char * const cmd[3] = {"fetchDict","-s", (char*)0};
        if ( execv( "/usr/bin/fetchDict", cmd ) < 0) {
            fprintf(stderr, "Execv error (mysql)\n");
            perror("Execv error(mysql):");
            exit(1);
        }
        printf("detectMouse.c (mysql)子进程已经退出...\n");
        exit(1);

    }

    pbcyan ( "Detect Mouse 退出: %d", getpid() );

    return NULL;
}

