#include "common.h"

extern char *shmaddr_google;
extern char *shmaddr_baidu;

char *baidu_result[BAIDUSIZE] = { NULL };
char *google_result[GOOGLESIZE] = { NULL };

/* 用于和detectMouse通信，当已经新建翻译结果显示窗口时，
 * 不再检测鼠标动作*/
int InNewWin = 0;

/* 用于切换百度和谷歌翻译结果的显示, -1和0互为取反运算*/
int show = -1;
int scrollShow = -1;

extern int action;

extern int lines_baidu;
extern int lines_google;

extern int maxlen_baidu;
extern int maxlen_google;

extern int strcatFlag;

extern char *text;

struct GtkText {

    GtkWidget *window;
    GtkLayout *layout;
    GtkWidget *button;

    GtkTextIter *iter;
    GtkTextBuffer *buf;

    int index_google[2];
    char **storage;
};

typedef struct WinData {

    GtkWidget *scroll;
    GtkWidget *button;
    GtkWidget *layout;
    GtkTextBuffer *gtbuf;
    GtkTextIter *iter;
    GtkWidget *window;

    gint width;
    gint height;

    gint hadShowGoogleResult;

}WinData;

typedef struct Google {

    double width;
    double height;
}Google;

Google gw;


typedef struct Baidu {

    double width;
    double height;
}Baidu;

Baidu bw;

int destroyNewWin(GtkWidget *window);
int waitForContinue();
void getIndex(int *index, char *addr);
void get_paragraph();
void initMemoryBaidu();
void initMemoryGoogle();
void printDebugInfo();
int  newScrolledWin();

void setFontProperties(GtkTextBuffer *buf, GtkTextIter *iter);

void changeDisplay(GtkWidget *button, gpointer *arg);
void displayGoogleTrans(GtkWidget *button, gpointer *arg);
void displayBaiduTrans(GtkTextBuffer *buf, GtkTextIter* iter, gpointer *arg);

void syncScrolledWinWithConfigEvent ( GtkWidget *window, GdkEvent *event, gpointer scroll );

void adjustDispalyForNewWin( GtkWidget *window, GdkEvent *event, gpointer scroll );

void adjustWinSize(GtkWidget *button, gpointer *arg, int which);

void setWinSizeForNewWin ( int maxlen, int lines, char *addr );

void showGoogleScrolledWin(GtkTextBuffer *gtbuf, GtkTextIter *iter, WinData *wd);

int getLinesOfGoogleTrans ( int *index_google );

GtkWidget *newSwitchButton() {

    /*创建按钮*/
    GtkWidget *button = gtk_button_new();
    GdkPixbuf *src = gdk_pixbuf_new_from_file("/home/rease/.stran/Switch.png", NULL);
    GdkPixbuf *dst = gdk_pixbuf_scale_simple(src, 20, 20, GDK_INTERP_BILINEAR);
    GtkWidget *image = gtk_image_new_from_pixbuf(dst);

    g_object_unref(src);
    g_object_unref(dst);

    gtk_button_set_image(GTK_BUTTON(button), image);

    return button;
}

int getLinesOfGoogleTrans ( int *index_google ) {

    int lines = 0;

    /* 显示时多了空行，index有值代表含有对应的结果，
     * 总共就是两行，lines+2*/
    for ( int i=0; i<2; i++ )
        if (index_google[i] != 0)
            lines += 2;

    /*长度小于30会显示原始输入，lines应该再加1*/
    if ( strlen(&shmaddr_google[ACTUALSTART]) < 30)
        lines++;

    return lines;
}

/*新建翻译结果窗口, 本文件入口函数*/
void *newWindow(void * arg) {

    show = -1;
    InNewWin = 1;

    printf("\n准备判断是否新建一般窗口\n\n");

    int ret = waitForContinue();

    if (ret == 1)
        return (void*)0;

    /*新建并设置窗口基本属性*/
    gtk_init(NULL, NULL);
    GtkWidget *newWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_keep_above(GTK_WINDOW(newWin), TRUE);
    gtk_window_set_title(GTK_WINDOW(newWin), "");
    gtk_window_set_position(GTK_WINDOW(newWin), GTK_WIN_POS_MOUSE);
    //gtk_window_set_resizable(GTK_WINDOW(newWin), FALSE);

    g_signal_connect(newWin, "destroy", G_CALLBACK(destroyNewWin), newWin);

    /*创建layout用于显示背景图片,以及放置文本*/
    GtkWidget * layout = gtk_layout_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(newWin), layout);

    /*创建scrolled window*/
    GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);

    /*建立文字显示区域*/
    GtkWidget *view;
    GtkTextBuffer *buf;
    GtkTextIter iter;

    view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);

    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_view_set_buffer((GtkTextView*)view, buf);

    /*设置离左边以及顶部的距离*/
    gtk_text_view_set_left_margin ( (GtkTextView*)view, 10 );
    gtk_text_view_set_top_margin ( (GtkTextView*)view, 10 );

    gtk_container_add (GTK_CONTAINER(scroll), view);
    gtk_layout_put(GTK_LAYOUT(layout), scroll, 0, 0);

    printDebugInfo();

    /*设置字体相关属性*/
    setFontProperties(buf, &iter);

    int index_google[2] = { 0 };
    int index_baidu[13] = { 0 };

    /*索引分隔符，存于index_xx[]*/
    if ( shmaddr_google[0] != ERRCHAR)
        getIndex(index_google, shmaddr_google);

    //TODO:if ( shmaddr_baidu[0] !=ERRCHAR )
    getIndex(index_baidu, shmaddr_baidu);

    printf("\033[0;36m百度翻译分隔符索引结果:\033[0m\n");
    for (int i=0; i<13; i++)
        printf("\033[0;33m%d \033[0m ", index_baidu[i]);
    printf("\n");

    /*初始化百度翻译结果存储空间*/
    if ( baidu_result[0] == NULL)
        initMemoryBaidu();

    /*分离数据时会统计这两个值，先置0*/
    maxlen_baidu = lines_baidu = 0;

    /* 从共享内存数据流中分离相关数据到baidu_result相关功能内存区域,
     * 第二个参数是单行显示字符长度*/
    separateData(index_baidu, 28);

    /*根据翻译结果的最大长度和行数设置窗口大小*/
    setWinSizeForNewWin ( maxlen_baidu, lines_baidu, shmaddr_baidu );

    printf("\033[0;33m");
    printf("(NewWin) baidu_maxlen=%d lines_baidu=%d \n",maxlen_baidu, lines_baidu);
    printf("\033[0m");

    /*根据前面得出的宽高设置窗口和scrolled window的宽高值*/
    gtk_window_set_default_size (GTK_WINDOW(newWin), bw.width, bw.height);
    gtk_widget_set_size_request (scroll, bw.width, bw.height);


    /*
     * 调整字符串代码段
     * */

    if ( google_result[0] == NULL )
        initMemoryGoogle();

    /*主要完成步骤:加入回车符使单行句子不至于太长*/
    if ( shmaddr_google[0]  != ERRCHAR ) {

        lines_google = 0;
        maxlen_google = 0;

        char *p[3];
        p[0] = &shmaddr_google[ACTUALSTART];
        p[1] = &shmaddr_google[index_google[0]];
        p[2] = &shmaddr_google[index_google[1]];

        adjustStr(p, 28, google_result);

        lines_google = getLinesOfGoogleTrans ( index_google );

        /* 如果只有单个翻译结果，在原始数据显示情况下，要计算其显示长度是否超过翻译结果的
         * 如果是，则替换之*/
        if ( index_google[0] == 0 && (ret = strlen (SourceInput)) < 30)
            if ( ret/2 > maxlen_google )
                maxlen_google = ret / 2;

        printf("\033[0;36mmaxlen_google=%d lines=%d34m\033[0m\n", maxlen_google,lines_google);

        /*存于全局变量*/
        gw.width = 14 * maxlen_google + 60;
        gw.height = lines_google * 26;

    }
    else  {
        shmaddr_google[0] = CLEAR;
        strcpy(google_result[0], "翻译超时或出现其他错误");
        lines_google = 2;

        gw.width = 250;
        gw.height = gw.width * 0.618;
        maxlen_google = 12;
    }

    /*创建切换按钮*/
    GtkWidget *button = newSwitchButton();

    gtk_layout_put(GTK_LAYOUT(layout), button, bw.width-50, bw.height-45);


    struct GtkText gt;

    gt.buf = buf;
    gt.iter = &iter;
    gt.index_google[0] = index_google[0];
    gt.index_google[1] = index_google[1];
    gt.layout = (GtkLayout*)layout;
    gt.button = button;
    gt.window = newWin;

    g_signal_connect(button, "clicked", G_CALLBACK(changeDisplay), (void*)&gt);

    /*显示百度翻译结果*/
    displayBaiduTrans(buf, &iter, (void*)&gt);

    WinData wd;

    wd.scroll = scroll;
    wd.button = button;
    wd.layout = layout;

    /*捕获resize window信号, 进行显示调整*/
    g_signal_connect (newWin, "configure-event", G_CALLBACK(adjustDispalyForNewWin), &wd);

    //gtk_widget_set_opacity(image, 0.7);
    //gtk_widget_set_opacity(view, 1);

    gtk_widget_show_all(newWin);
    gtk_main();

    pthread_exit(NULL);
}

int destroyNewWin(GtkWidget *window) {

    maxlen_baidu = 0;
    maxlen_google = 0;
    lines_baidu = 0;
    lines_google = 0;

    memset(shmaddr_google, '0', 10);
    memset(shmaddr_baidu, '0', 10);

    memset(shmaddr_google, '\0', SHMSIZE-10);
    memset(shmaddr_baidu, '\0', SHMSIZE-10);

    /* TODO:谷歌结果存放空间未填充0*/
    for ( int i=0; i<BAIDUSIZE; i++ )
        memset( baidu_result[i], '\0', SHMSIZE / BAIDUSIZE );

    /*清除相关标记*/
    shmaddr_baidu[0] = CLEAR;
    shmaddr_google[0] = CLEAR;

    //gtk_window_close(GTK_WINDOW(window));
    gtk_widget_destroy(window);
    gtk_main_quit();

    /* TODO:按了exit键后变成了单击事件，此时再双击会导致检测错误
     * 应手动置0 ( 当前可以不用这个了，这是以前用过的,不过先放着
     * 可能以后用得着，可以当个提醒 )*/
    action = 0;

    /* 已退出翻译结果窗口，重置标志变量*/
    InNewWin = 0;

    return FALSE;
}

/*Get index of separate symbols*/
void getIndex(int *index, char *addr) {

    char *p = &addr[ACTUALSTART];
    int i = ACTUALSTART;  /*同p一致指向同一个下标字符*/
    int charNum = 0;

    while ( *p ) 
    {
        if ( *p == '|' ) 
        {
            *p = '\0';
            if ( addr == shmaddr_google && charNum >= 2 ) /*截取到第三个分隔符*/ {
                printf("已完成分割字符索引\n");
                break;
            }

            index[charNum++] = i + 1; /*记录字符串下标*/
        }
        p++; i++;
    }

    /* 清除翻译结果写入完成标志*/
    if ( addr[0] == '1')
        addr[0] = '\0';
}

int waitForContinue() {

    int flag = 0;
    int time = 0;

    /*等待任意一方python端的翻译数据全部写入共享内存*/
    while( shmaddr_google[0] != FINFLAG && shmaddr_baidu[0] != FINFLAG ) {

        if ( flag ) {
            flag = 0;
            printf("准备接收共享内存数据...\n");
        }

        /*长时间未检测到共享内存里的数据进行双击取消本次窗口显示*/
        if ( action == DOUBLECLICK ) {

            printf("捕获双击退出: In newWindow.c\n");

            /*action==DOUBLECLICK只代表取消显示，应重置action*/
            action = 0;
            return 1;
        }
        if ( shmaddr_google[0] == ERRCHAR) {
            printf("翻译过程出现错误, 准备窗口错误提示\n");
            break;
        }

        if ( shmaddr_google[0] == NULLCHAR || shmaddr_google[0] == EXITFLAG) {
            if ( shmaddr_google[0] == NULLCHAR)
                printf("空字符串\n");
            else {
                printf("Note: 翻译程序程序已退出,请不要再往下执行\n");
                printf("退出程序...\n");
                exit(0);
            }
            action = 0;
            shmaddr_google[0] = CLEAR;
            shmaddr_baidu[0] = CLEAR;
            InNewWin = 0;
            return 1;
        }
        usleep(400000);
        time++;

        /* 超时时间1.6S ( 2 * 400000ms )*/
        if ( time >= 2 ) {
            shmaddr_google[0] = ERRCHAR;
            shmaddr_baidu[0] = ERRCHAR;
            break;
        }
    }

    if ( shmaddr_baidu[0] == EXITFLAG ) {
        fprintf(stderr, "\033[0;35m百度翻译异常退出...\n\033[0m");
        fprintf(stderr, "\033[0;35m正在停止取词翻译...\n\033[0m");
        quit();
    }

    action = 0;

    /*原始数据超过一定长度，在ScrolledWin中显示, 并返回1，
     * 不再执行newWin函数*/
    if ( strlen (text) > 130 ) 
        return newScrolledWin();

    return 0;
}

/* 初始化百度翻译结果存储空间*/
void initMemoryBaidu() {

    if (baidu_result[0] != NULL)
        return;

    for (int i=0; i<BAIDUSIZE; i++) {

        baidu_result[i] = calloc(SHMSIZE / BAIDUSIZE, sizeof ( char ) );
        if (baidu_result[i] == NULL)
            err_exit("Error occured when calloc memory in initMemoryBaidu");
    }
}

/* 类上*/
void initMemoryGoogle() {

    if (google_result[0] != NULL)
        return;

    for (int i=0; i<GOOGLESIZE; i++) {

        google_result[i] = calloc(SHMSIZE / GOOGLESIZE, sizeof ( char ) );
        if (google_result[i] == NULL)
            err_exit("Error occured when calloc memory in initMemoryGoogle");
    }
}

/* 重新从共享内存获取百度翻译结果并设置窗口大小*/
void reGetBaiduTransAndSetWin (gpointer *arg, int which ) { 

    printf("\033[0;36m重新从共享内存获取百度翻译数据 \033[0m\n");
    maxlen_baidu = 0;
    lines_baidu = 0;

    int index_baidu[13] = { 0 };
    for ( int i=0; i<13; i++ )
        printf("\033[0;36m%d \033[0m ", index_baidu[i]);
    printf("\n");

    printDebugInfo();
    getIndex(index_baidu, shmaddr_baidu);

    separateData(index_baidu, 28);

    setWinSizeForNewWin ( maxlen_baidu, lines_baidu, shmaddr_baidu );

    //adjustWinSize ( ((struct GtkText*)arg)->button, arg, which );

    printf("\033[0;36mmaxlen=%d lines_baidu=%d bw.width=%f bw.height=%f\033[0m\n", maxlen_baidu, \
            lines_baidu, bw.width, bw.height);

}

void adjustWinSize(GtkWidget *button, gpointer *arg, int which) {

    if ( !which ) 
    {
        int index[2] = { 0 }, ret = 0;
        getIndex(index, shmaddr_google);

        char *p[3];
        p[0] = &shmaddr_google[ACTUALSTART];
        p[1] = &shmaddr_google[index[0]];
        p[2] = &shmaddr_google[index[1]];

        adjustStr(p, 28, google_result);

        lines_google = getLinesOfGoogleTrans ( index );

        /* 如果只有单个翻译结果，在原始数据显示情况下，要计算其显示长度是否超过翻译结果的
         * 如果是，则替换之*/
        if ( index[0] == 0 && (ret = strlen (SourceInput)) < 30)
            if ( ret/2 > maxlen_google )
                maxlen_google = ret / 2;

        printf("\033[0;33mmaxlen_google=%d lines=%d\033[0m\n", maxlen_google,lines_google);

        /*存于全局变量*/
        gw.width = 14 * maxlen_google + 60;
        gw.height = lines_google * 26;

        /* TODO：mark1*/
        if ( index[0] != 0)
            setWinSizeForNewWin ( maxlen_google, lines_google, shmaddr_google );

        /*如果新窗口的宽高都小于上一个的，不调整窗口大小*/
        if ( gw.width < bw.width && gw.height < bw.height)
            return;

        if (gw.width < 350)
            gw.width = 350;

        if ( gw.height < 200 )
            gw.height = 200;

        printf("\033[0;33mgw width=%f height=%f\033[0m\n", gw.width, gw.height);

        gtk_window_resize((GtkWindow*)((struct GtkText*)arg)->window, gw.width, gw.height);
        gtk_layout_move((GtkLayout*)((struct GtkText*)arg)->layout, button, gw.width-50, gw.height-45);
        gtk_widget_show_all(((struct GtkText*)arg)->window);
    } 

    else 
    {
        /*还未获取到结果，应重新获取并设置窗口大小*/
        if ( strlen ( ZhTrans ) == 0) {

            printf("\033[0;36m百度翻译结果长度为0\033[0m\n");
            reGetBaiduTransAndSetWin ( arg, which );
        }

        /*如果新窗口的宽高都小于上一个的，不调整窗口大小*/
        if ( gw.width > bw.width && gw.height > bw.height)
            return;

        if ( bw.width < 300 )
            bw.width = 300;

        if ( bw.height <= 0 )
            bw.height = bw.width * 0.618;

        printf("\033[0;35mbw width=%f %f\033[0m\n", bw.width, bw.height);
        gtk_window_resize((GtkWindow*)((struct GtkText*)arg)->window, bw.width, bw.height);
        gtk_layout_move((GtkLayout*)((struct GtkText*)arg)->layout, button, bw.width-50, bw.height-45);
        gtk_widget_show_all(((struct GtkText*)arg)->window);
    }
}

/* 切换百度和谷歌翻译结果的显示*/
void changeDisplay(GtkWidget *button, gpointer *arg) {

    show = ~show;

    adjustWinSize ( button, arg, show );

    if ( show ) {
        displayBaiduTrans( ((struct GtkText*)arg)->buf,((struct GtkText*)arg)->iter, arg);
    }
    else 
        displayGoogleTrans(button, arg);
}

void displayGoogleTrans(GtkWidget *button, gpointer *arg) {

    printf("\033[0;33m\n显示谷歌翻译结果:\033[0m\n\n");

    /* 调整窗口大小*/
    adjustWinSize ( button, arg, 0 );

    GtkTextIter *iter, start, end;
    iter = ((struct GtkText*)arg)->iter;
    GtkTextBuffer *buf = ((struct GtkText*)arg)->buf;

    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_get_end_iter(buf, &end);

    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);

    int index_google[2] = { 0 };

    /* 比较字符串是否相等,如果不相等，说明用于谷歌翻译结果存储的共享内存被改写了，
     * 需要重新分离调整字符串*/
    if (strcmp ( &shmaddr_google[ACTUALSTART], google_result[0] ) != 0) {

        printf("\033[0;31m字符串不相等: google_result[0]->%s< \033[0m\n\n", google_result[0]);
        printf("ACTUALSTART=>%s<\n",&shmaddr_google[ACTUALSTART] );

        for ( int i=0; i<2; i++ ) {
            printf("\033[0;35m%d \033[0m\n",((struct GtkText*)arg)->index_google[i] );
            if ( shmaddr_google[((struct GtkText*)arg)->index_google[i] - 1] == '\0')
                shmaddr_google[((struct GtkText*)arg)->index_google[i] - 1] = '|';
        }

        printf("\033[0;35m添加分隔符后字符串:>%s< \033[0m\n", &shmaddr_google[ACTUALSTART]);

        lines_google = 0;

        index_google[0] = index_google[1] = 0;
        getIndex(index_google, shmaddr_google);
        printf("\n\033[0;32mindex:%d %d\n\033[0m", index_google[0], index_google[1]);

        char *p[3] ={ NULL };
        p[0] = &shmaddr_google[ACTUALSTART];
        p[1] = &shmaddr_google[index_google[0]];
        p[2] = &shmaddr_google[index_google[1]];

        adjustStr(p, 28, google_result);

    } else {
        printf("\033[0;33m字符串依然相等\033[0m\n");

        /*若字符串依旧相等，直接拿来用就行*/
    }

    char doubleEnter[] = "\n\n";
    char enter[] = "\n";

    /*插入输入原文*/
    if ( strlen( text )  < 30 ) {

        gtk_text_buffer_insert_with_tags_by_name(buf, iter, text, -1, 
                "black-font", "gray_background", "bold-style", "Uneditable", "font-size-15", "underline", NULL);

        gtk_text_buffer_insert_with_tags_by_name(buf, iter, doubleEnter, -1, NULL, NULL);
    }

    /*插入翻译结果*/
    for ( int i=0; i<3; i++ ) {

        printf("\033[0;33mgoogle_result[%d]=%s\033[0m\n", i, google_result[i]);
        if ( google_result[i][0] != '\0') {

            gtk_text_buffer_insert_with_tags_by_name(buf, iter, google_result[i], -1, 
                    "green-font", "gray_background", "bold-style", "Uneditable", "font-size-11", NULL);

            if ( i<2 && google_result[i+1][0] != '\0')
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, doubleEnter, -1, NULL, NULL);
            else 
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
        }
    }
}
void displayBaiduTrans(GtkTextBuffer *buf, GtkTextIter* iter, gpointer *arg) {

    printf("\033[0;36m显示百度翻译结果\033[0m\n");

    if ( strlen (ZhTrans) == 0 && strlen ( EnTrans ) == 0 ) {

        printf("\033[0;36mZhTrans & EnTrans长度皆为0 \033[0m\n");

        reGetBaiduTransAndSetWin ( arg, -1 );
    }

    GtkTextIter start, end;

    /*找到开头和结尾并删除，重新定位到初始为位置0*/
    gtk_text_buffer_get_end_iter(buf, &end);
    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);

    char enter[] = "\n";

    /*根据得到的相关结果进行翻译内容输出*/
    for ( int i=0; i<BAIDUSIZE-1; i++ ) {

        /* 翻译结果不为空*/
        if ( baidu_result[i][0] != '\0') {

            /* 翻译结果输出控制代码段*/
            if ( i == 0 && strlen(baidu_result[i]) < 30 )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i], -1,\
                        "black-font", "gray_background", "bold-style", "Uneditable",\
                        "font-size-15", "letter-spacing","underline", NULL);
            else if ( i == 1 )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i], -1, 
                        "blue-font", "gray_background", "heavy-font", "Uneditable",\
                        "font-size-11", "letter-spacing", NULL);
            else if ( i == 4 )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i], -1, 
                        "brown-font", "gray_background", "heavy-font", "Uneditable",\
                        "font-size-11","letter-spacing", NULL);
            else if ( i != 0 )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i], -1, 
                        "green-font", "gray_background", "heavy-font", "Uneditable",\
                        "font-size-11", "letter-spacing", NULL);


            /* 回车符控制输出代码段*/
            if ( 0 && i == 0 && ( strlen(baidu_result[3]) ==0 && strlen(baidu_result[4]) == 0\
                        && strlen(baidu_result[2]) != 0 && strlen(baidu_result[1]) == 0))

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);

            else if ( i == 1 && strlen(baidu_result[1]) != 0 && ( strlen(baidu_result[2]) != 0\
                        || strlen(baidu_result[3]) != 0 || strlen(baidu_result[4]) != 0))

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);

            else if ( i == 2 && ( strlen(baidu_result[3]) != 0 || strlen(baidu_result[4]) != 0 ) )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);


            else if ( i == 3 && ( strlen(baidu_result[4]) != 0) )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
        } 

        /* 翻译结果检测为空*/
        else if ( i == 0 ){

            gtk_text_buffer_insert_with_tags_by_name(buf, iter, "尚未获取到数据,请尝试刷新\n", -1,\
                    "black-font", "gray_background", "bold-style", "Uneditable",\
                    "font-size-15", "letter-spacing","underline", NULL);


            /* 一般来说谷歌翻译的结果获取快一点，如果百度翻译此时还没获取到，
             * 先返回谷歌翻译的结果, 显示顺序改变，需同步show*/
            show = ~show;
            printf("\033[0;31m百度翻译结果为空，尚未成功获取，重定向到谷歌翻译结果 \033[0m\n");
            return displayGoogleTrans ( ((struct GtkText*)arg)->button, arg);
        }
    }

}

void setFontProperties(GtkTextBuffer *buf, GtkTextIter *iter) {

    /*注意属性值设置正确，不然桌面分分钟崩溃:(*/

    gtk_text_buffer_create_tag(buf, "black-font", "foreground", "#000000", NULL);
    gtk_text_buffer_create_tag(buf, "yellow-font", "foreground", "#c8ab02", NULL);
    gtk_text_buffer_create_tag(buf, "blue-font", "foreground", "#00aaff", NULL);
    gtk_text_buffer_create_tag(buf, "brown-font", "foreground", "#aaaa7f", NULL);
    gtk_text_buffer_create_tag(buf, "green-font", "foreground", "#216459", NULL);
    gtk_text_buffer_create_tag(buf, "bold-style", "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag(buf, "heavy-font", "weight", PANGO_WEIGHT_HEAVY, NULL);
    gtk_text_buffer_create_tag(buf, "Uneditable", "editable", FALSE, NULL);
    gtk_text_buffer_create_tag(buf, "font-size-10", "font", "10", NULL );
    gtk_text_buffer_create_tag(buf, "font-size-11", "font", "11", NULL );
    gtk_text_buffer_create_tag(buf, "font-size-12", "font", "12", NULL );
    gtk_text_buffer_create_tag(buf, "font-size-13", "font", "13", NULL );
    gtk_text_buffer_create_tag(buf, "font-size-14", "font", "14", NULL );
    gtk_text_buffer_create_tag(buf, "font-size-15", "font", "15", NULL );
    gtk_text_buffer_create_tag(buf, "font-size-17", "font", "17", NULL );
    gtk_text_buffer_create_tag(buf, "gray_background", "background", "#ffffff", NULL);
    gtk_text_buffer_create_tag(buf, "letter-spacing", "letter-spacing", 100, NULL);
    gtk_text_buffer_create_tag(buf, "underline", "underline", PANGO_UNDERLINE_SINGLE, NULL);

    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);
}

void printDebugInfo() {

    printf("\n\033[0;36mFinish标志位: %c\033[0m", shmaddr_baidu[0]);
    printf("\n\033[0;36mPhonetic标志位: %c\033[0m", shmaddr_baidu[1]);
    printf("\n\033[0;36mNumbers of zhTrans标志位: %c\033[0m", shmaddr_baidu[2]);
    printf("\n\033[0;36mNumbers of enTrans标志位: %c\033[0m", shmaddr_baidu[3]);
    printf("\n\033[0;36mOther forms of word标志位: %c\033[0m", shmaddr_baidu[4]);
    printf("\n\033[0;36mNumbers of audio links标志位: %c\033[0m\n", shmaddr_baidu[5]);

    printf("\n\033[0;36m百度翻译结果: %s\n\n\033[0m", &shmaddr_baidu[ACTUALSTART]);
    printf("\033[0;33m谷歌翻译结果: %s\033[0m\n\n", &shmaddr_google[ACTUALSTART]);
}

/*窗口重绘以及自动调整switch button*/
void adjustDispalyForNewWin( GtkWidget *window, GdkEvent *event, gpointer data ) {

    gint width, height;
    gtk_window_get_size ( (GtkWindow*)window, &width, &height );
    gtk_widget_set_size_request ( (GtkWidget*)((WinData*)data)->scroll,  width, height);
    gtk_layout_move ( (GtkLayout*)((WinData*)data)->layout, ((WinData*)data)->button, \
            width-50, height-45 );
    gtk_widget_queue_draw ( window );
}

/*跟上一个函数是一样的, 这里主要是考虑到后期拓展和区分窗口写成了两个函数*/
void syncScrolledWinWithConfigEvent ( GtkWidget *window, GdkEvent *event, gpointer wd ) {

    gint width, height;
    gtk_window_get_size ( (GtkWindow*)window, &width, &height );
    gtk_widget_set_size_request ( (GtkWidget*)((WinData*)wd)->scroll,  width, height);
    gtk_layout_move ( (GtkLayout*)((WinData*)wd)->layout, ((WinData*)wd)->button, width-50, height-45 );
    gtk_widget_queue_draw ( window );
}

void resizeScrolledWin ( WinData *wd, gint width, gint height ) {

    gtk_window_resize ((GtkWindow*) wd->window, width, height );
    gtk_widget_set_size_request ( wd->scroll,  width, height);
    gtk_layout_move ( (GtkLayout*)wd->layout, wd->button, width-50, height-45 );
    gtk_widget_queue_draw ( wd->window );

}

/* Suit the size of window with the number of characters */
void suitWinSizeWithCharNum ( char *addr , WinData *wd) {

    if ( addr == shmaddr_baidu ) {

        int charNums = countCharNums ( &shmaddr_baidu[ACTUALSTART+2] );

        printf("\033[0;35m (showBaiduScrolledWin) charNums=%d\n", charNums);

        /* 根据字符数量控制窗口大小和单行长度*/
        if ( charNums < 400 ) {

            int lines = countLines ( 30 , &shmaddr_google[ACTUALSTART] );
            printf("\033[0;35m (showBaiduScrolledWin) lines=%d\n", lines);
            adjustStrForScrolledWin ( 30, &shmaddr_baidu[2+ACTUALSTART] );
            wd->width = 600;
            wd->height = lines * 35;

            if ( wd->height < 300 )
                wd->height = 300;
        } 
        else  {

            int lines = countLines ( 30 , &shmaddr_google[ACTUALSTART] );
            wd->width = 900;
            wd->height = lines * 20;

            if ( wd->height < 300 )
                wd->height = 300;

            adjustStrForScrolledWin ( 46, &shmaddr_baidu[ACTUALSTART+2] );
            strcpy ( ZhTrans,  &shmaddr_baidu[ACTUALSTART+2]);
        }
    }

    if ( addr == shmaddr_google ) {

        int index[2] = { 0 };
        getIndex(index, shmaddr_google);

        int charNums = countCharNums ( &shmaddr_google[ACTUALSTART] );

        /* 根据字符数量控制窗口大小和单行长度*/
        if ( charNums < 400 ) {

            int lines = countLines ( 30 , &shmaddr_google[ACTUALSTART] );
            adjustStrForScrolledWin( 30, &shmaddr_google[ACTUALSTART] );
            wd->width = 600;
            wd->height = lines * 35;
            if ( wd->height < 300 )
                wd->height = 300;
        }
        else {

            int lines = countLines ( 30 , &shmaddr_google[ACTUALSTART] );
            wd->width = 900;
            wd->height = lines * 20;
            if ( wd->height < 300 )
                wd->height = 300;
            adjustStrForScrolledWin( 46, &shmaddr_google[ACTUALSTART] );
        }

    }
}

void showBaiduScrolledWin(GtkTextBuffer *gtbuf, GtkTextIter *iter, WinData *wd) {

    printf("\033[0;36m(showBaiduScrolledWin)百度翻译结果:%s\n\033[0m", &shmaddr_baidu[ACTUALSTART]);

    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter (gtbuf, &start);
    gtk_text_buffer_get_end_iter (gtbuf, &end);
    gtk_text_buffer_delete (gtbuf, &start, &end);
    gtk_text_buffer_get_iter_at_offset (gtbuf, iter, 0);

    int index[13] = { 0 };
    getIndex(index, shmaddr_baidu);

    printf("\033[0;36m(showBaiduScrolledWin)百度翻译结果分隔符索引: \033[0m");
    for ( int i=0; i<4; i++ )
        printf("%d ", index[i]);
    printf("\n");

    /* getIndex 会将分隔符修改为'\0', 再一次进入这个函数index[1]会成0
     * 所以这里是为了区分是否是第一次进入这里，如果不是，插入已经保存
     * 到ZhTrans的数据, 否则先生成ZhTrans*/
    if ( index[1] != 0 ) {

        shmaddr_baidu[0] = '0';

        printf("\033[0;36m (showBaiduScrolledWin) width=%d height=%d\n", wd->width, wd->height);

        suitWinSizeWithCharNum ( shmaddr_baidu , wd );

        resizeScrolledWin ( wd, wd->width, wd->height );

        strcpy ( ZhTrans,  &shmaddr_baidu[index[1]]);

        printf("\033[0;31m 调整后输出的百度翻译结果:%s\033[0m\n", ZhTrans);

        gtk_text_buffer_insert_with_tags_by_name ( gtbuf, iter, ZhTrans, -1,\
                "green-font", "font-size-14", "bold-style", NULL );
    }

    else if ( strlen ( ZhTrans ) != 0 ) {

        //adjustStrForScrolledWin ( 30, &shmaddr_baidu[ACTUALSTART+2] );
        //strcpy ( ZhTrans,  &shmaddr_baidu[ACTUALSTART+2]);

        /* 窗口大了，对显示的字符串进行相应调整, 扩大单行显示长度为46*/
        if ( wd->width >= 900 ) {

            adjustStrForScrolledWin ( 46, &shmaddr_baidu[ACTUALSTART+2] );
            strcpy ( ZhTrans,  &shmaddr_baidu[ACTUALSTART+2]);
        }

        resizeScrolledWin ( wd, wd->width, wd->height );

        printf("\033[0;36m ZhTrans里的结果输出\033[0m\n");

        gtk_text_buffer_insert_with_tags_by_name ( gtbuf, iter, ZhTrans, -1,\
                "green-font", "font-size-14", "bold-style", NULL );
    }
    else {

        printf("\033[0;31m 未获取到百度翻译结果\033[0m\n");

        gtk_text_buffer_insert_with_tags_by_name ( gtbuf, iter, "未获取到百度翻译结果\n", -1,\
                "green-font", "font-size-14", "bold-style", NULL );

        return (void)showGoogleScrolledWin (gtbuf, iter, wd);
    }
}

int isContainSeparateChar ( char *str ) {

    if ( str == NULL || str[0] == '\0')
        return 0;

    char *p = str;

    while ( *p ) {
        if ( *p++ == '|' )
            return 1;
    }

    return 0;

}

void showGoogleScrolledWin(GtkTextBuffer *gtbuf, GtkTextIter *iter, WinData *wd) {

    printf("\033[0;33m谷歌翻译结果:%s\033[0m\n\n", &shmaddr_google[ACTUALSTART]);

    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter (gtbuf, &start);
    gtk_text_buffer_get_end_iter (gtbuf, &end);
    gtk_text_buffer_delete (gtbuf, &start, &end);
    gtk_text_buffer_get_iter_at_offset (gtbuf, iter, 0);

    /* 仍旧包含分隔符，说明是新的字符串翻译结果，未显示，需要调整*/
    if ( isContainSeparateChar ( &shmaddr_google[ACTUALSTART] ) )
        wd->hadShowGoogleResult = 0;

    if ( ! wd->hadShowGoogleResult ) {

        printf("\033[0;31mHadn't show google result \033[0m\n");

        suitWinSizeWithCharNum ( shmaddr_google, wd );

        resizeScrolledWin ( wd, wd->width, wd->height );

        wd->hadShowGoogleResult  = 1;
    }

    gtk_text_buffer_insert_with_tags_by_name ( gtbuf, iter, &shmaddr_google[ACTUALSTART], -1,\
            "green-font", "font-size-14", "bold-style", NULL );
}

/* 切换百度谷歌翻译结果显示*/
void switchScrolledWin(GtkWidget *button, gpointer data) {

    scrollShow= ~scrollShow;
    if ( scrollShow ) {
        showBaiduScrolledWin(((WinData*)data)->gtbuf, ((WinData*)data)->iter, (WinData*)data);
        return;
    }

    showGoogleScrolledWin(((WinData*)data)->gtbuf, ((WinData*)data)->iter, (WinData*)data);
}

int  newScrolledWin() {

    GtkWidget *window;

    InNewWin = 1;

    gint width = 900+20;
    gint height;

    int lines = 0, charNums = 0, ret = 0;

    lines = countLines(46, &shmaddr_google[ACTUALSTART]);
    charNums = countCharNums(&shmaddr_google[ACTUALSTART]);

    if ( ( ret = countCharNums(&shmaddr_baidu[2]) ) > charNums)
        charNums = ret;

    if ( charNums <= 500 )
        height = 450;

    printf("\033[0;34m new scrolled window: lines=%d\033[0m\n", lines);
    printf("\033[0;34m new scrolled window: characterNums=%d\033[0m\n", charNums);

    height = lines * 30;

    if ( height < 400 )
        height = 400;

    if ( height > 900 )
        height = 900;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW(window), width, height);
    gtk_window_set_title (GTK_WINDOW(window), "STRAN");

    GtkWidget *layout = gtk_layout_new (NULL, NULL);
    GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);
    GtkWidget *view = gtk_text_view_new();

    gtk_text_view_set_editable ( (GtkTextView*)view, FALSE );
    gtk_text_view_set_cursor_visible ( ( GtkTextView* )view, FALSE );

    gtk_container_add ( GTK_CONTAINER(scroll), view );
    gtk_container_add ( GTK_CONTAINER(layout), scroll );
    gtk_container_add (GTK_CONTAINER(window), layout);

    gtk_widget_set_size_request ( scroll,  width, height);

    GtkTextBuffer *gtb = gtk_text_view_get_buffer((GtkTextView*)view);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), 10);
    gtk_text_view_set_top_margin ( (GtkTextView*)view, 10 );

    GtkWidget *button = newSwitchButton();
    gtk_layout_put ( (GtkLayout*)layout, button, width-50, height-45 );

    GtkTextIter iter;

    WinData wd;
    wd.layout = layout;
    wd.button = button;
    wd.scroll = scroll;
    wd.gtbuf = gtb;
    wd.width = width;
    wd.height = height;
    wd.iter = &iter;
    wd.window = window;
    wd.hadShowGoogleResult = 0;

    g_signal_connect (window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect (window, "configure-event", G_CALLBACK(syncScrolledWinWithConfigEvent), &wd);
    g_signal_connect (button, "clicked", G_CALLBACK(switchScrolledWin), &wd);

    setFontProperties (gtb, &iter);

    scrollShow = -1;

    /*初始化内存*/
    if ( baidu_result[0] == NULL)
        initMemoryBaidu();

    showBaiduScrolledWin(gtb, &iter, &wd);

    gtk_widget_show_all(window);

    gtk_main();

    strcatFlag = 1;
    InNewWin = 0;

    memset ( ZhTrans, '\0', SHMSIZE / BAIDUSIZE );

    pthread_exit(NULL);

    return 1;
}


/* 设置newWin的窗口大小*/
void setWinSizeForNewWin ( int maxlen, int lines, char *addr ) {

    int ret = 0;

    if ( addr == shmaddr_baidu ) {

        /*来个黄金比例的矩形*/
        double width=310.0, height = width * 0.618;

        /*宽度为20时差不多是长310，由此得出此公式*/
        width = (width / 18) * maxlen;

        /*只有一个中文翻译结果，高度适当设置的小一点,行数×10就行*/
        if ( NumZhTranFlag == 1 && NumEnTranFlag == 0 && PhoneticFlag ==0\
                && OtherWordFormFlag == 0 && lines> 2) {

            if ( (ret = strlen (SourceInput)) < 30)
                maxlen= ret / 2;

            height = ( lines+3 ) * 10 + 40;

        }
        else
            height = ( lines+3 ) * 22 ;

        /*别让窗口过小*/
        if ( width < 300 ) {
            width = 300;
            height = 140;
        }

        bw.width = width;
        bw.height = height;

        return;
    } 

    /* mark1*/
    else if ( addr == shmaddr_google ) 
    {
    }
}
