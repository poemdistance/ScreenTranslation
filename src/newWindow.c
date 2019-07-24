#include "common.h"

extern int InNewWinFunc;
extern char *shmaddr;
extern int action;
extern int HadDestroied;
extern int CanNewWin;

int destroy_newwin_by_clicked(GtkWidget *button, GtkWidget *window);
int destroy_newwin_by_destroy(GtkWidget *window);

/*新建翻译结果窗口*/
void *newWindow(void * arg) {

    int num;

    while ( CanNewWin != 1 ) {
        printf("wait for create new window.., CanNewWin=%d\n", CanNewWin);
        usleep(200000);
        num++;
        if ( num > 10 )
            pthread_exit(NULL);
    }

    /* 置零action，用于下面翻译窗口弹不出时可以双击关闭*/
    action = 0; 
    InNewWinFunc = 1;

    printf("new window func in newWindow.c\n");

    /*等待python端的翻译数据全部写入共享内存*/
    while( shmaddr[0] != '1') {

        //printf("准备接收共享内存数据...\n");

        /*长时间未检测到共享内存里的数据进行双击或者单机取消本次窗口显示*/
        if ( action == DOUBLECLICK/* || action == SINGLECLICK*/) {

            printf("捕获双击退出: In newWindow.c\n");

            /*此处action只代表取消显示，应重置action*/
            action = 0;
            InNewWinFunc = 0;
            return (void*)0;
        }

        if ( shmaddr[0] == '2' ) {
            printf("翻译过程出现错误\n");
            action = 0;
            InNewWinFunc = 0;
            shmaddr[0] = '0';
            return (void*)0;
        }
        usleep(400000);
    }

    /*新建并设置窗口基本属性*/
    gtk_init(NULL, NULL);
    GtkWidget *newWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_keep_above(GTK_WINDOW(newWin), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(newWin), 250, 170);
    gtk_window_set_title(GTK_WINDOW(newWin), "");
    gtk_window_set_position(GTK_WINDOW(newWin), GTK_WIN_POS_MOUSE);

    /*新建box容纳文字显示*/
    GtkWidget *lbox;
    lbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(newWin), lbox);

    g_signal_connect(newWin, "destroy", G_CALLBACK(destroy_newwin_by_destroy), newWin);

    /*建立文字显示区域*/
    GtkWidget *view;
    GtkTextBuffer *buf;
    view = gtk_text_view_new();
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_view_set_buffer((GtkTextView*)view, buf);
    gtk_container_add(GTK_CONTAINER(lbox), view);

    gtk_widget_show_all(newWin);
    gtk_main();

    CanNewWin = 0;
    printf("new window quit...\n");

    pthread_exit(NULL);
}


/* 发现在用结构体传值后强制转换类型前相关变量就进行了类型检测
 * 导致发生错误, 只能规规矩矩传进未修改的变量GtkWidget *window */
int destroy_newwin_by_clicked(GtkWidget *button, GtkWidget *window) {

    gtk_widget_destroy(window);
    gtk_widget_destroy(button);
    gtk_main_quit();

    /*标记已退出newWindow函数*/
    InNewWinFunc = 0;
    printf("memset shmaddr in clicked\n");
    memset(shmaddr, '\0', SHMSIZE);

    /* 按了exit键后变成了单击事件，此时再双击会导致检测错误
     * 应手动置0*/
    printf("Set action = 0\n");
    action = 0;
    return TRUE;
}

int destroy_newwin_by_destroy(GtkWidget *window) {

    /*标记已退出newWindow函数*/
    InNewWinFunc = 0;
    printf("memset shmaddr in destroy\n");
    memset(shmaddr, '\0', SHMSIZE);

    //gtk_window_close(GTK_WINDOW(window));
    gtk_widget_destroy(window);
    gtk_main_quit();

    /* 按了exit键后变成了单击事件，此时再双击会导致检测错误
     * 应手动置0*/
    action = 0;
    return FALSE;
}
