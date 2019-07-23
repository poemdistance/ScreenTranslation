/* 注意：
 * 程序退出暂时没有销毁共享内存
 * 等项目后期再来规划相关问题*/

#include <gtk/gtk.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include "common.h"

#define GETEKYDIR ("/tmp")
#define PROJECTID  (2333)
#define SHMSIZE (1024)

struct timeval start, end;
char *shmaddr;
int action = 0;
int hideIcon = 0;
int startDetect = 0;
int EnterNotToHide = 0;
int HadDestroied = 0;
int InNewWinFunc = 0;
int TranWin = 0;
int timeout_id_1;
int timeout_id_2;
pthread_t GUIID;

struct Arg {
    int argc;
    char **argv;
    char *addr;
};

struct clickDate {
    GtkWidget *window;
    GtkWidget *button;
};

char *text = NULL;
FILE *fp = NULL;
int mousefd;
int fd_key = -1;

/* 发现在用结构体传值后强制转换类型前相关变量就进行了类型检测
 * 导致发生错误, 只能规规矩矩传进未修改的变量GtkWidget *window */
int destroy_newwin(GtkWidget *button, GtkWidget *window) {

    gtk_widget_destroy(window);
    gtk_widget_destroy(button);
    gtk_main_quit();

    /*标记已退出newWindow函数*/
    InNewWinFunc = 0;
    printf("memset shmaddr\n");
    memset(shmaddr, '\0', SHMSIZE);

    /* 按了exit键后变成了单击事件，此时再双击会导致检测错误
     * 应手动置0*/
    printf("Set action = 0\n");
    action = 0;
    return FALSE;
}

/*新建翻译结果窗口*/
int newWindow(GtkWidget *widget, GtkWidget *window) {

    /* 置零action，用于下面翻译窗口弹不出时
     * 可以双击关闭*/
    action = 0; 
    InNewWinFunc = 1;

    printf("new window func\n");

    while( shmaddr[0] != '1') {
        if ( action == DOUBLECLICK || action == SINGLECLICK) {

            printf("destroy old win\n");
            gtk_widget_destroy(window);
            gtk_widget_destroy(widget);
            gtk_main_quit();

            /*此处action只代表取消显示，应重置action*/
            action = 0;
            InNewWinFunc = 0;
            HadDestroied = 1;
            return -1;
        }
        usleep(400);
    }

    shmaddr[0] = '\0';

    printf("destroy old win\n");
    gtk_widget_destroy(window);
    gtk_widget_destroy(widget);

    HadDestroied = 1;

    printf("new window\n");
    GtkWidget *newWin;
    gtk_init(NULL, NULL);
    newWin = gtk_window_new(GTK_WINDOW_POPUP);

    gtk_window_set_default_size(GTK_WINDOW(newWin), 250, 170);
    gtk_window_set_title(GTK_WINDOW(newWin), "");
    gtk_window_set_position(GTK_WINDOW(newWin), GTK_WIN_POS_MOUSE);

    /*box*/
    GtkWidget *lbox;
    //lbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    lbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(newWin), lbox);

    /*放置退出按钮到指定位置*/
    GtkWidget *fixed;
    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(lbox), fixed);

    GtkWidget *button = gtk_button_new_with_label("exit");
    gtk_widget_set_size_request(button, 10,6);
    gtk_fixed_put(GTK_FIXED(fixed), button, 0, 0);
    g_signal_connect(button, "clicked", G_CALLBACK(destroy_newwin), newWin);
    g_signal_connect(newWin, "destroy", G_CALLBACK(gtk_main_quit), newWin);

    /*建立文字显示区域*/
    //GtkWidget *view;
    //GtkTextBuffer *buf;
    //view = gtk_text_view_new();
    //buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    //gtk_text_view_set_buffer((GtkTextView*)view, buf);

    char *p = &shmaddr[1];
    while(*p++) {
        if ( *p == '\n' )
            *p = '.';

        if ( *p == '|')
            *p = '\n';
    }
    //gtk_text_buffer_set_text(buf, &shmaddr[3], -1);
    //gtk_fixed_put(GTK_FIXED(fixed), view, 0, 40);

    GtkWidget *btnTxt = gtk_button_new_with_label(&shmaddr[1]);
    gtk_fixed_put(GTK_FIXED(fixed), btnTxt, 3, 90);

    gtk_widget_show_all(newWin);
    gtk_main();

    gtk_main_quit();

    return TRUE;
}


int quit_test(void *arg) {

    struct clickDate * cd;
    cd = (struct clickDate*)arg;
    GtkWidget *button = cd->button;
    GtkWidget *window = cd->window;

    printf("单击检测\n");

    printf("action=%d, HadDestroied=%d\n", action, HadDestroied);
    if ( HadDestroied ) {
        g_source_remove(timeout_id_2);
        return FALSE;
    }

    if (!HadDestroied && (action == SINGLECLICK) && window) {

        if ( action == SINGLECLICK ) {
            printf("单击销毁\n");
            action = 0;
            HadDestroied = 1;
            gtk_widget_destroy(button);
            gtk_widget_destroy(window);
            gtk_main_quit();
            return FALSE;
        }
    }

    return TRUE;
}

int quit_entry(void *arg) {

    struct clickDate * cd;
    cd = (struct clickDate*)arg;
    GtkWidget *button = cd->button;
    GtkWidget *window = cd->window;

    printf("超时检测\n");
    if ( HadDestroied ) {
        g_source_remove(timeout_id_1);
        return FALSE;
    }

    if ( button &&  window && !HadDestroied ) {

        printf("超时销毁\n");
        gtk_widget_destroy(button);
        gtk_widget_destroy(window);
        printf("set action = -1\n");
        action  = -1;
        g_source_remove(timeout_id_1);
        gtk_main_quit();
        return FALSE;
    }

    return TRUE;
}

void *GUI(void *arg) {

    int ret = 1;
    int ret2 = 1;
    printf("Enter GUI Function\n");

    while(1) {
        if( ret == 1 || ret2 == 1) {
            ret = g_source_remove(timeout_id_1);
            ret2 = g_source_remove(timeout_id_2);
        }
        usleep(500);
        if ( action == DOUBLECLICK || action == SLIDE  ) {
            printf("Detect mouse action, creating icon entry\n");
            shmaddr[0] = '\0';
            break;
        }
    }

    GtkWidget *window;

    /*入口图标销毁标志*/
    HadDestroied = 0;

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_POPUP);

    gtk_window_set_title(GTK_WINDOW(window), "");
    gtk_window_set_default_size(GTK_WINDOW(window), 10,10);
    gtk_window_set_deletable(GTK_WINDOW(window), FALSE); 
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_TOOLBAR); 
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE); 
    GtkWidget *button = gtk_button_new();


    /*TODO:添加文件存在性检测*/
    GtkWidget *image = gtk_image_new_from_file("./tran.png");
    gtk_button_set_image(GTK_BUTTON(button), image);
    gtk_container_add(GTK_CONTAINER(window), button);
    gtk_widget_set_app_paintable(window, TRUE);
    gtk_widget_set_opacity(window, 0.7);
    GdkScreen *screen = gtk_widget_get_screen(window);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
    gtk_widget_set_visual(window, visual);


    g_signal_connect(button, "clicked",\
            G_CALLBACK(newWindow), window);

    gint cx, cy;
    gtk_window_get_position(GTK_WINDOW(window), &cx, &cy);
    gtk_window_move(GTK_WINDOW(window), cx+20, cy-60);
    gtk_widget_show_all(window);

    struct clickDate cd;
    cd.window = window;
    cd.button = button;
    timeout_id_1 = g_timeout_add(2000, quit_entry, &cd);
    timeout_id_2 = g_timeout_add(600, quit_test, &cd);

    gtk_main();

    printf("GUI prhread_exit...\n");
    pthread_exit(NULL);
}

void *DetectMouse(void *arg) {

    printf("Detect Mouse\n");
    struct sigaction sa;
    int retval ;
    char buf[3];
    char appName[100];
    int releaseButton = 1;
    time_t current;
    fd_set readfds;
    struct timeval tv;
    struct timeval old, now;
    double oldtime = 0;
    double newtime = 0;
    double lasttime = 0;
    int thirdClick;
    int lastAction=0;

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
                //printf("%d %d %d %d\n", history[m], history[n]);

                /*没有按下按键并活动鼠标,标志releaseButton=1*/
                if ( history[m] == 0 && history[n] == 0 ) {
                    releaseButton = 1;
                    action = 0;
                }

                /*按下左键*/
                if ( history[m] == 1 && history[n] == 0 ) {
                    if ( releaseButton ) {
                        time(&current);
                        gettimeofday(&old, NULL);

                        /* lasttime为双击最后一次的按下按键时间;
                         * 如果上次双击时间到现在不超过600ms，则断定为3击事件;
                         * 3击会选中一整段，或一整句，此种情况也应该复制文本*/
                        if (abs(lasttime - ((old.tv_usec + old.tv_sec*1000000) / 1000)) < 600 \
                                && lasttime != 0)
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

                /*检测双击时间间隔*/
                if ( isAction(history, i, DOUBLECLICK) )  {
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

                    fprintf(stdout, "got appName = %s", appName);

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

                    writePipe(text, fd[1]);

                    lastAction = action;

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
        if ( execv( "/usr/bin/tranen", cmd ) < 0) {
            fprintf(stderr, "Execv error\n");
            exit(1);
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    printf("pid=%d\n",getpid ());
    pthread_t t1, t2, t3;
    int no1, no2;
    struct Arg arg;

    key_t key = ftok(GETEKYDIR, PROJECTID);
    if ( key < 0 )
        err_exit("ftok error");
    printf("key=%d\n", key);

    int shmid;
    shmid = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if ( shmid == -1 ) {
        if ( errno == EEXIST ) {
            printf("shared memeory already exist\n");
            shmid = shmget(key ,0, 0);
            printf("reference shmid = %d\n", shmid);
        } else {
            perror("errno");
            err_exit("shmget error");
        }
    } else {
        printf("shmid=%d\n", shmid);
    }

    char *addr;

    /* Do not to specific the address to attach
     * and attach for read & write*/
    if ( (addr = shmat(shmid, 0, 0) ) == (void*)-1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
            err_exit("shmctl error");
        else {
            printf("Attach shared memory failed\n");
            printf("remove shared memory identifier successful\n");
        }

        err_exit("shmat error");
    } else {
        printf("Attach to %p\n", addr);
    }

    shmaddr = addr;

    arg.addr = addr;
    arg.argc = argc;
    arg.argv = argv;

    /*启动检测鼠标动作的线程*/
    pthread_create(&t2, NULL, DetectMouse, (void*)&arg);

    void *thread_ret;
    GUIID = t1;

    while (1) {

        pthread_create(&t1, NULL, GUI, (void*)&arg);
        pthread_join(t1, &thread_ret);

        /*进入newWindown函数时不再创建入口图标线程*/
        while(InNewWinFunc == 1)
            usleep(300);

        printf("---------------------------GUI thread exit----------------------\n");

        GUIID = t1;
    }

    /*TODO:
     * The following codes will not be executed,
     * remember to handle it*/
    pthread_join(t2, &thread_ret); 

    if ( shmdt(addr) < 0)
        err_exit("shmdt error");

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        err_exit("shmctl error");
    else {
        printf("Finally\n");
        printf("remove shared memory identifier successful\n");
    }

}
