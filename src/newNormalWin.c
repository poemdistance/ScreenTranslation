/* 注意不要频繁调用gtk_widget_queue_draw(),
 * 否则越靠后的重绘指令越有可能不被成功执行*/

#include "common.h"
#include "newWindow.h"
#include "audio.h"
#include "cleanup.h"

#define SHMADDR(who)  ( ( who ) == BAIDU ? (shmaddr_baidu) : ( shmaddr_mysql ) )
#define TYPE(who)     ( ( who ) == BAIDU ? ( ONLINE ) : ( OFFLINE ))
#define WIDTH(who)    ( ( who ) == BAIDU ? ( bw.width ): ( mw.width ) )
#define HEIGHT(who)    ( ( who ) == BAIDU ? ( bw.height ): ( mw.height ) )
#define WHO(addr)     ( ( addr ) == shmaddr_mysql ? OFFLINE : BAIDU )

#define STORAGE_WIDTH(who,value) if(who==BAIDU) {\
    bw.width=value;}\
    else {\
        mw.width=value;}

#define STORAGE_HEIGHT(who,value) if(who==BAIDU) {\
    bw.height=value;}\
    else {\
        mw.height=value;}

#define PRINT_WHO(who) \
    if(who==BAIDU){\
        printf("  BAIDU\n");\
    }\
    else {\
        printf(" OFFLINE\n");}

char *baidu_result[BAIDUSIZE] = { NULL };
char *google_result[GOOGLESIZE] = { NULL };
char *mysql_result[MYSQLSIZE] = { NULL };

/* 用于和detectMouse通信，当已经新建翻译结果显示窗口时，
 * 不再检测鼠标动作*/
int InNewWin = 0;

/* 用于切换百度和谷歌翻译结果的显示, -1和0互为取反运算*/
int show = -1;

/* 鼠标动作标志位*/
extern int action;

extern int CanNewWin;

static int focustimes = 0;

int timeout_id = 0;

/* 本函数代码借鉴自xdotool部分源码*/
void focusOurWindow( WinData *wd ) {

    /* Get window id of x11*/
    GdkWindow *gw = gtk_widget_get_window ( GTK_WIDGET ( wd->window ) );
    Window wid = gdk_x11_window_get_xid ( gw );

    /* Get window's attributes*/
    XWindowAttributes wattr;
    Display *dpy = XOpenDisplay (NULL);
    if ( !dpy ) 
        return;

    XGetWindowAttributes(dpy, wid, &wattr);

    XEvent xev;
    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.display = dpy;
    xev.xclient.window = wid;
    xev.xclient.message_type = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 2L; /* 2 == Message from a window pager */
    xev.xclient.data.l[1] = CurrentTime;

    int ret = XSendEvent(dpy, wattr.screen->root, False,
            SubstructureNotifyMask | SubstructureRedirectMask,
            &xev);

    if ( ret == 0 ) {
        pred("窗口聚焦请求失败(focusOurWindow)");
    }

    XCloseDisplay(dpy);

}

int detect_ctrl_c(void *data) {

    /* 每隔一定时间多次尝试重新聚焦窗口防止聚焦窗口被抢占*/
    if ( focustimes++ <= 5 ) {
        focusOurWindow ( WINDATA(data) );
    }


    /* 此处监听的是电脑全局，如果希望其他地方的ctrl-c
     * 使本翻译界面关闭，请注释掉下面代码中的: < 0 & >*/

    /* Ctrl_C事件标志位, 赋值位置 captureShortcutEvent.c <变量shmaddr>*/
    if ( 0 & (shmaddr_keyboard[CTRL_C_PRESSED_FLAG ] == '1') ) {

        pyellow("窗口检测到Control-C");

        WinData *wd = WINDATA(data);

        gtk_window_present_with_time ( GTK_WINDOW(wd->window), time(NULL) );
        gtk_window_set_focus ( (GtkWindow*)wd->window, wd->view );
        gtk_window_set_focus ( (GtkWindow*)wd->window, wd->scroll );
        destroyNormalWin(wd->window, wd);
        return FALSE;
    }

    return TRUE;
}


void text_changed ( GtkTextBuffer *buf, gpointer *data ) {

    if ( (WINDATA(data))->lineHeight > 0 )
        return;

    gtk_widget_show((WINDATA(data))->window);

    /*
       int charnum = gtk_text_buffer_get_char_count ( buf );
       GtkTextIter iter ;
       gtk_text_buffer_get_iter_at_offset ( buf, &iter, charnum );
       */
    GtkTextIter start , end;
    GdkRectangle loc;
    gtk_text_buffer_get_start_iter ( buf, &start);
    gtk_text_buffer_get_end_iter ( buf, &end );
    //gtk_text_view_get_iter_location ( GTK_TEXT_VIEW((WINDATA(data))->view), &iter, &loc );
    gtk_text_view_get_iter_location ( GTK_TEXT_VIEW((WINDATA(data))->view), &end, &loc );
    (WINDATA(data))->lineHeight = loc.y;
}


/*新建翻译结果窗口, 本文件入口函数*/
void *newNormalWindow() {

    /* Storage the relative element or data in this window*/
    WinData wd;

    /*Important: Pay attention to clear the values the global variables*/
    bw.width = 400;
    bw.height = 300;
    mw.width = 400;
    mw.height = 300;
    gw.width = 400;
    gw.height = 300;

    focustimes = 1;
    show = -1;
    InNewWin = 1;

    printf("\n准备判断是否新建一般窗口\n\n");

    /* 窗口打开标志位 changed in captureShortcutEvent.c <变量shmaddr>*/
    shmaddr_keyboard[WINDOW_OPENED_FLAG] = '1';

    wd.getOfflineTranslation = 0;
    wd.specific = 0;

    int ret = waitForContinue( &wd );

    if (ret == 1)
        return (void*)0;

    /*新建并设置窗口基本属性*/
    gtk_init(NULL, NULL);
    GtkWidget *newWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_keep_above(GTK_WINDOW(newWin), TRUE);
    gtk_window_set_title(GTK_WINDOW(newWin), "");
    gtk_window_set_accept_focus ( GTK_WINDOW(newWin), TRUE );
    gtk_window_set_focus_on_map ( GTK_WINDOW(newWin), TRUE );
    gtk_widget_set_can_focus(newWin, TRUE);

    /* quickSearch快捷键标志位, changed in captureShortcutEvent.c <变量shmaddr>*/
    if ( shmaddr_keyboard[QuickSearchShortcutPressed_FLAG] == '1') {
        shmaddr_keyboard[QuickSearchShortcutPressed_FLAG ] = '0';

        /* 快捷键调出的窗口放置于中央*/
        gtk_window_set_position(GTK_WINDOW(newWin), GTK_WIN_POS_CENTER);
    }
    else
        /* 取词翻译的窗口跟随鼠标*/
        gtk_window_set_position(GTK_WINDOW(newWin), GTK_WIN_POS_MOUSE);

    //gtk_window_set_resizable(GTK_WINDOW(newWin), FALSE);

    g_signal_connect(newWin, "destroy", G_CALLBACK(destroyNormalWin), &wd);

    /*创建layout用于显示背景图片,以及放置文本*/
    GtkWidget * layout = gtk_layout_new(NULL, NULL);

    /*创建scrolled window*/
    GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);

    /*建立文字显示区域*/
    GtkWidget *view;
    GtkTextBuffer *buf;
    GtkTextIter iter;

    /* 隐藏文字区域的光标并关闭可编辑属性*/
    view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);

    gtk_window_present ( GTK_WINDOW(newWin) );
    gtk_widget_grab_focus ( view );
    gtk_window_set_focus ( GTK_WINDOW(newWin), view );

    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_view_set_buffer((GtkTextView*)view, buf);


    /*设置离左边以及顶部的距离*/
    gtk_text_view_set_left_margin ( (GtkTextView*)view, 10 );
    gtk_text_view_set_top_margin ( (GtkTextView*)view, 10 );

    /* window->layout->scroll->view*/
    gtk_container_add (GTK_CONTAINER(scroll), view);
    gtk_container_add (GTK_CONTAINER(layout), scroll);
    gtk_container_add (GTK_CONTAINER(newWin), layout);

    setFontProperties(buf, &iter);

    wd.image =  NULL;
    wd.oldImage = NULL;
    wd.view = view;
    wd.window = newWin;
    wd.layout = layout;
    wd.srcBackgroundImage = NULL;
    wd.gdkwin = NULL;
    wd.width = 400;
    wd.height = 300;
    wd.lineHeight = 0;
    wd.phonPos = 0;
    wd.hadRedirect = 0;
    wd.forceResize = 0;

    /* Signals are used by everyone, but they are only created on a per class basis \
     * -- so you should not call call gtk_signal_new() unless you are writing a new \
     *  GtkObject type. However, if you want to make a new signal for an existing \
     *  type, you may use gtk_object_class_user_signal_new() to create a signal that\
     *  doesn't correspond to a class's builtin methods.
     * */

    /* 监听键盘事件的回调函数*/
    g_signal_connect ( newWin, "key-press-event", G_CALLBACK(key_press), (void*)&wd );

    /* 这个获取背景图片并适应窗口的代码段放在分离翻译数据之前耗的时间才有
     * 意义，因为图片加载虽然花了一定时间，但总的还是会比翻译结果获取成功
     * 要少很多, 放在前面可以等到数据全部成功获取(一般来说百度的翻译结果会
     * 获取的比较慢)*/
    wd.image = syncImageSize ( newWin, wd.width, wd.height, (void*)&wd );

    printDebugInfo();

    /*初始化百度以及离线翻译结果存储空间*/
    initMemoryBaidu();
    initMemoryMysql();

    gtk_window_set_default_size (GTK_WINDOW(newWin), bw.width, bw.height);
    gtk_widget_set_size_request ( layout, bw.width, bw.height );
    gtk_widget_set_size_request (scroll, bw.width, bw.height);

    int index_google[2] = { 0 };

    /*索引分隔符，存于index_xx[]*/
    if ( shmaddr_google[0] != ERRCHAR)
        getIndex(index_google, shmaddr_google);

    if ( google_result[0] == NULL )
        initMemoryGoogle();

    /* 分离谷歌翻译共享内存数据,并保存应设窗口大小值*/
    separateGoogleDataSetWinSize ( index_google );

    /* 百度翻译结果显示按钮*/
    GtkWidget *baiduButton = newBaiduButton( &wd );
    gtk_layout_put ( GTK_LAYOUT(layout), baiduButton, bw.width-RIGHT_BORDER_OFFSET, bw.height-BOTTOM_OFFSET );
    g_signal_connect(baiduButton, "clicked", G_CALLBACK(displayBaiduTrans), &wd);

    /* 谷歌翻译结果显示按钮*/
    GtkWidget *googleButton = newGoogleButton ( &wd );
    gtk_layout_put ( GTK_LAYOUT(layout), googleButton, bw.width-RIGHT_BORDER_OFFSET*2, bw.height-BOTTOM_OFFSET );
    g_signal_connect ( googleButton, "clicked", G_CALLBACK(displayGoogleTrans), &wd );

    /* 离线翻译结果切换按钮*/
    GtkWidget *offlineButton = newOfflineButton ( &wd );
    gtk_layout_put ( GTK_LAYOUT(layout), offlineButton, bw.width-RIGHT_BORDER_OFFSET*3, bw.height-BOTTOM_OFFSET );
    g_signal_connect ( offlineButton, "clicked", G_CALLBACK(displayOfflineTrans), &wd );

    /* 指示当前翻译所属*/
    wd.indicateButton = newIndicateButton ( &wd );
    gtk_layout_put ( GTK_LAYOUT(layout), wd.indicateButton, bw.width-RIGHT_BORDER_OFFSET, bw.height-INDICATE_OFFSET );

    wd.buf = buf;
    wd.iter = &iter;
    wd.volume = NULL;
    wd.scroll = scroll;

    wd.index_google[0] = index_google[0];
    wd.index_google[1] = index_google[1];


    /*捕获resize window信号, 进行显示调整*/
    g_signal_connect (newWin, "configure-event", G_CALLBACK(syncNormalWinForConfigEvent), &wd);


    timeout_id = g_timeout_add(RIGHT_BORDER_OFFSET*2, detect_ctrl_c, &wd);

    gtk_widget_show_all(newWin);

    /* GtkTextView 插入文本时的回调函数注册*/
    //g_signal_connect ( buf, "changed", G_CALLBACK(text_changed), (void*)&wd );

    /*显示百度翻译结果*/
    displayBaiduTrans(baiduButton, (void*)&wd );

    gtk_main();

    pthread_exit(NULL);
}

int destroyNormalWin(GtkWidget *window, WinData *wd) {

    clearMemory();

    g_source_remove ( timeout_id );

    /* 窗口关闭标志位*/
    shmaddr_keyboard[WINDOW_OPENED_FLAG] = '0';
    shmaddr_keyboard[CTRL_C_PRESSED_FLAG] = '0';

    //gtk_widget_destroy ( wd->image );

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
                break;
            }

            index[charNum++] = i + 1; /*记录字符串下标*/
        }
        p++; i++;
    }

    /*Can somebody tell me why? 清除翻译结果写入完成标志*/
    if ( addr[0] == '1')
        addr[0] = '\0';
}

int waitForContinue(WinData *wd) {

    int flag = 0;
    int time = 0;

    /*等待任意一方python端的翻译数据全部写入共享内存*/
    while(/* (CanNewWin != 1) || */ ( shmaddr_google[0] != FINFLAG && shmaddr_baidu[0] != FINFLAG \
                && shmaddr_mysql[0] != FINFLAG )) {


        if ( flag ) {
            flag = 0;
            printf("准备接收共享内存数据...\n");
        }

        /*长时间未检测到共享内存里的数据进行双击取消本次窗口显示*/
        if ( action == DOUBLECLICK ) {

            printf("捕获双击退出: In newNormalWindow.c\n");

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
            pred("超时退出");
            shmaddr_google[0] = ERRCHAR;
            shmaddr_baidu[0] = ERRCHAR;
            break;
        }

    }

    if ( text == NULL )
        if (( text = calloc(TEXTSIZE, 1)) == NULL)
            err_exit("malloc failed in notify.c");

    if ( shmaddr_baidu[0] == EXITFLAG ) {
        pred("百度翻译异常退出");
        pred("正在停止取词翻译");
        quit();
    }

    action = 0;

    if ( shmaddr_mysql[0] == FINFLAG )
        wd->getOfflineTranslation = 1;

    /*原始数据超过一定长度，在ScrolledWin中显示, 并返回1，
     * 不再执行newWin函数*/
    if ( strlen (text) > 130 ) 
        return newScrolledWin();

    return 0;
}
/* 重新从共享内存获取百度翻译结果并设置窗口大小*/
void reGetBaiduTransAndSetWin (gpointer *data, int who ) { 

    pred("重新从共享内存获取百度翻译数据");

    PRINT_WHO(who);

    int index[13] = { 0 };

    //printDebugInfo();

    getIndex(index, SHMADDR(who) );

    separateDataForBaidu(index, 28, TYPE(who) );

    setWinSizeForNormalWin (WINDATA(data), SHMADDR(who), TYPE(who));

    pgreen("(reGetBaiduTrans) width=%f height=%f", WIDTH(who), HEIGHT(who));

    //WINDATA(data)->width = WIDTH(who);
    //WINDATA(data)->height = HEIGHT(who);
}

void adjustWinSize(GtkWidget *button, gpointer *data, int who ) {

    if ( who == GOOGLE ) 
    {
        int index[2] = { 0 };

        pred("谷歌翻译重新索引 index:");
        getIndex(index, shmaddr_google);

        /* 找到分割符，数据分离提取才有意义*/
        if ( index[0] != 0 )
            separateGoogleDataSetWinSize ( index );
        else
            pred("未找到分隔符(adjustWinSize)");

        if (gw.width < 400)
            gw.width = 400;
        else if ( gw.width > 1000 )
            gw.width = 1000;

        if ( gw.height < 230 )
            gw.height = 230;
        else if ( gw.height > 900 )
            gw.height = 900;

        /* 更新较大值*/
        if ( gw.width > WINDATA(data)->width )
            WINDATA(data)->width = gw.width;
        if ( gw.height > WINDATA(data)->height )
            WINDATA(data)->height = gw.height;
    }
    else if ( who == BAIDU || who == OFFLINE)
    {
        /*还未获取到结果，应重新获取并设置窗口大小*/
        if ( strlen ( ZhTrans(TYPE(who)) ) == 0) {

            pred("百度翻译结果长度为0 (adjustWinSize)\n");
            reGetBaiduTransAndSetWin ( data, who );
        }

        /*如果新窗口的宽高都小于上一个的，不调整窗口大小*/
        if (  WIDTH(who) <= WINDATA(data)->width && HEIGHT(who) <= WINDATA(data)->height ) {

            pgreen("应设窗口小于当前，不必调整\n");
            return;
        }

        pcyan("WIDTH(who)=%f %f\n", WIDTH(who), HEIGHT(who));
    }

    WINDATA(data)->specific = 1;
    pbred("Call sync window funciton");
    syncNormalWinForConfigEvent( WINDATA(data)->window, NULL, data );
}

int nextWindow ( int who ) {

    if ( who < 3 )
        return who+1;

    return 1;
}

/* 切换各个翻译结果的显示*/
void changeDisplay(GtkWidget *button, gpointer *data) {

    WINDATA(data)->who = nextWindow((WINDATA(data)->who));

    adjustWinSize ( button, data, WINDATA(data)->who );

    if ( WINDATA(data)->who == BAIDU )
        displayBaiduTrans( WINDATA(data)->baiduButton, data );
    else if ( WINDATA(data)->who == GOOGLE )
        displayGoogleTrans(button, data);
    else if ( WINDATA(data)->who == OFFLINE )
        displayOfflineTrans(button, data);
}

void displayGoogleTrans(GtkWidget *button, gpointer *data) {

    WINDATA(data)->who = GOOGLE;
    WINDATA(data)->specific = 1;

    /* 调整窗口大小*/
    adjustWinSize ( button, data, GOOGLE );

    gint width = WINDATA(data)->width;
    gint height = WINDATA(data)->height;

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,(WINDATA(data))->indicateButton,\
            width-(200-(RIGHT_BORDER_OFFSET*(WINDATA(data))->who)), height-INDICATE_OFFSET );
    gtk_widget_queue_draw( WINDATA(data)->window );

    pbyellow("current indicate size %d %d",width, height );
    gtk_window_get_size((GtkWindow*)WINDATA(data)->window, &width, &height);
    pbyellow("current window size: %d %d", width, height);

    pyellow("\n显示谷歌翻译结果:\n\n");

    if ( WINDATA(data)->volume != NULL )
        gtk_widget_hide ( WINDATA(data)->volume  );

    GtkTextIter *iter, start, end;
    iter = WINDATA(data)->iter;
    GtkTextBuffer *buf = WINDATA(data)->buf;

    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_get_end_iter(buf, &end);

    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);

    WINDATA(data)->iter = iter ;

    int index_google[2] = { 0 };

    /* 比较字符串是否相等,如果不相等，说明用于谷歌翻译结果存储的共享内存被改写了，
     * 需要重新分离调整字符串*/
    if ( strcmp ( &shmaddr_google[ACTUALSTART], google_result[0] ) != 0 ) {

        pred("字符串不相等: google_result[0]->%s< \n\n", google_result[0]);
        printf("ACTUALSTART=>%s<\n",&shmaddr_google[ACTUALSTART] );

        for ( int i=0; i<2; i++ ) {
            if ( shmaddr_google[WINDATA(data)->index_google[i] - 1] == '\0') {
                shmaddr_google[WINDATA(data)->index_google[i] - 1] = '|';
            }
        }

        index_google[0] = index_google[1] = 0;
        getIndex(index_google, shmaddr_google);

        /* index_google[0] == 0没有索引到分隔符，不必调整字符，不然反而会出错*/
        if ( index_google[0] != 0) {

            char *p[3] ={ NULL };
            p[0] = &shmaddr_google[ACTUALSTART];
            p[1] = &shmaddr_google[index_google[0]];
            p[2] = &shmaddr_google[index_google[1]];

            adjustStr(p, 28, google_result);
        }

    } else {
        pgreen("字符串依然相等");

        /*若字符串依旧相等，直接拿来用就行*/
    }

    pred("Call syncImageSize in disGoogle \n");

    /* mark*/
    syncImageSize ( WINDATA(data)->window, WINDATA(data)->width,WINDATA(data)->height,  data) ;

    char enter[] = "\n";

    /*插入输入原文*/
    if ( strlen( text )  < 30 ) {

        gtk_text_buffer_insert_with_tags_by_name(buf, iter, text, -1, 
                "black-font",  "bold-style",  "font-size-15", "underline", NULL);

        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
        //gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
    }

    pyellow("displayGoogleTrans: \n");
    /*插入翻译结果*/
    for ( int i=0; i<3; i++ ) {

        if ( google_result[i][0] != '\0') {

            if ( i == 0 ) {

                if ( google_result[1][0] == '\0'  || google_result[2][0] == '\0') {

                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                }

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, google_result[i], -1, 
                        "brown-font",  "bold-style",  "font-size-11", NULL);

                if ( google_result[1][0] != '\0'  || google_result[2][0] != '\0') {

                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                }
            }
            else if ( i == 1 ) {

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, google_result[i], -1, 
                        "green-font",  "bold-style",  "font-size-11", NULL);
            }
            else if ( i == 2 )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, google_result[i], -1, 
                        "brown-font",  "bold-style",  "font-size-11", NULL);

            gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
        }
    }
}

/* 离线翻译结果展示窗口, 跟displayBaiduTrans重复较多*/
void displayOfflineTrans ( GtkWidget *button, gpointer *data ) {

    pmag("显示离线翻译:\n");

    WINDATA(data)->who = OFFLINE;
    WINDATA(data)->specific = 1;

    adjustWinSize ( button, data, OFFLINE );

    gint width = WINDATA(data)->width;
    gint height = WINDATA(data)->height;

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,(WINDATA(data))->indicateButton,\
            width-(200-(RIGHT_BORDER_OFFSET*(WINDATA(data))->who)), height-INDICATE_OFFSET );
    gtk_widget_queue_draw( WINDATA(data)->window );

    pbyellow("current indicate size %d %d",width, height );
    gtk_window_get_size((GtkWindow*)WINDATA(data)->window, &width, &height);
    pbyellow("current window size: %d %d", width, height);

    GtkTextIter *iter, start, end;
    iter = WINDATA(data)->iter;
    GtkTextBuffer *buf = WINDATA(data)->buf;

    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_get_end_iter(buf, &end);

    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);

    WINDATA(data)->iter = iter ;

    if ( ! WINDATA(data)->getOfflineTranslation && strlen ( ZhTrans(OFFLINE) ) == 0 )
        gtk_text_buffer_insert_with_tags_by_name(buf, iter, "\n  NOT FOUND ANYTHING",\
                -1, "yellow-font",  "heavy-font", \
                "font-size-11", "letter-spacing", NULL);

    syncVolumeBtn ( WINDATA(data), OFFLINE );

    char enter[] = "\n";

    /*根据得到的相关结果进行翻译内容输出*/
    for ( int i=0; i<BAIDUSIZE-1; i++ ) {

        /* 翻译结果不为空*/
        if ( mysql_result[i][0] != '\0') {

            /* 翻译结果输出控制代码段*/
            if ( i == 0 && strlen(mysql_result[i]) < 30 ) {

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, mysql_result[i], \
                        -1,"black-font",  "bold-style", \
                        "font-size-15", "letter-spacing","underline", NULL);
            }
            else if ( i == 1 ) {

                gtk_text_buffer_get_end_iter ( buf, &end );

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, mysql_result[i],\
                        -1, "blue-font",  "heavy-font", \
                        "font-size-11", "letter-spacing", NULL);

            }
            else if ( i == 4 ) {

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, mysql_result[i],\
                        -1, "brown-font",  "heavy-font", \
                        "font-size-11","letter-spacing", NULL);
            }
            else if ( i != 0 ) {

                if ( strlen(Phonetic(OFFLINE)) == 0 && strlen(ZhTrans(OFFLINE)) != 0\
                        && strlen(EnTrans(OFFLINE)) == 0 && strlen(OtherWordForm(OFFLINE)) == 0){

                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, mysql_result[i],\
                            -1, "brown-font",  "heavy-font", \
                            "font-size-11", "letter-spacing", NULL);

                } else {

                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, mysql_result[i],\
                            -1, "green-font",  "heavy-font", \
                            "font-size-11", "letter-spacing", NULL);
                }

            }

            /* 回车符控制输出代码段*/
            if ( i == 0 )  {

                /* 只有源输入而之后没结果了，插入一个回车符*/
                if (( strlen(mysql_result[3]) ==0 && strlen(mysql_result[4]) == 0\
                            && strlen(mysql_result[2]) == 0 && strlen(mysql_result[1]) == 0)) 
                {
                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                }

                else  
                {   
                    if (!(strlen(Phonetic(OFFLINE)) == 0 && strlen(ZhTrans(OFFLINE)) != 0 && strlen(EnTrans(OFFLINE)) == 0\
                                && strlen(OtherWordForm(OFFLINE)) == 0 && strlen(ZhTrans(OFFLINE)) != 0) ){
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                    }
                    else
                    {
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                    }
                }
            }

            else if ( i == 1 && strlen(mysql_result[1]) != 0 && ( strlen(mysql_result[2]) != 0\
                        || strlen(mysql_result[3]) != 0 || strlen(mysql_result[4]) != 0))

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);

            else if ( i == 2 && ( strlen(mysql_result[3]) != 0 || strlen(mysql_result[4]) != 0 ) )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);


            else if ( i == 3 && ( strlen(mysql_result[4]) != 0) )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
        } 
    }

}


/* Baidu online translation display window */
void displayBaiduTrans(GtkWidget *button,  gpointer *data ) {

    pgreen("显示百度翻译:");

    WINDATA(data)->who = BAIDU;
    WINDATA(data)->specific = 1;

    adjustWinSize ( button, data, BAIDU );

    gint width = WINDATA(data)->width;
    gint height = WINDATA(data)->height;

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,(WINDATA(data))->indicateButton,\
            width-(200-(RIGHT_BORDER_OFFSET*(WINDATA(data))->who)), height-INDICATE_OFFSET );

    gtk_widget_queue_draw( WINDATA(data)->window );

    pbyellow("current indicate size %d %d",width, height );
    gtk_window_get_size((GtkWindow*)WINDATA(data)->window, &width, &height);
    pbyellow("current window size: %d %d", width, height);

    GtkTextBuffer *buf = WINDATA(data)->buf;

    if ( strlen (ZhTrans(ONLINE)) == 0 && strlen ( EnTrans(ONLINE) ) == 0 ) {

        pred("ZhTrans(ONLINE) & EnTrans(ONLINE)长度皆为0 \n");
        reGetBaiduTransAndSetWin ( data, BAIDU );
    }

    GtkTextIter start, end, *iter;
    iter = WINDATA(data)->iter;

    /*找到开头和结尾并删除，重新定位到初始为位置0*/
    gtk_text_buffer_get_end_iter(buf, &end);
    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);

    WINDATA(data)->iter = iter ;

    syncImageSize ( WINDATA(data)->window,WINDATA(data)->width,WINDATA(data)->height, data) ;
    syncVolumeBtn ( WINDATA(data), ONLINE );

    char enter[] = "\n";

    /*根据得到的相关结果进行翻译内容输出*/
    for ( int i=0; i<BAIDUSIZE-1; i++ ) {

        /* 翻译结果不为空*/
        if ( baidu_result[i][0] != '\0') {

            /* 翻译结果输出控制代码段*/
            if ( i == 0 && strlen(baidu_result[i]) < 30 ) {

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i], \
                        -1,"black-font",  "bold-style", \
                        "font-size-15", "letter-spacing","underline", NULL);
            }
            else if ( i == 1 ) {

                gtk_text_buffer_get_end_iter ( buf, &end );

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i],\
                        -1, "blue-font",  "heavy-font", \
                        "font-size-11", "letter-spacing", NULL);
            }
            else if ( i == 4 ) {

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i],\
                        -1, "brown-font",  "heavy-font", \
                        "font-size-11","letter-spacing", NULL);
            }
            else if ( i != 0 ) {

                if ( strlen(Phonetic(ONLINE)) == 0 && strlen(ZhTrans(ONLINE)) != 0\
                        && strlen(EnTrans(ONLINE)) == 0 && strlen(OtherWordForm(ONLINE)) == 0){

                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i],\
                            -1, "brown-font",  "heavy-font", \
                            "font-size-11", "letter-spacing", NULL);

                } else {

                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i],\
                            -1, "green-font",  "heavy-font", \
                            "font-size-11", "letter-spacing", NULL);
                }

            }

            /* 回车符控制输出代码段*/
            if ( i == 0 )  {

                /* 只有源输入而之后没结果了，插入一个回车符*/
                if (( strlen(baidu_result[3]) ==0 && strlen(baidu_result[4]) == 0\
                            && strlen(baidu_result[2]) == 0 && strlen(baidu_result[1]) == 0)) 
                {
                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                }

                else  
                {   
                    if (!(strlen(Phonetic(ONLINE)) == 0 && strlen(ZhTrans(ONLINE)) != 0 && strlen(EnTrans(ONLINE)) == 0\
                                && strlen(OtherWordForm(ONLINE)) == 0 && strlen(ZhTrans(ONLINE)) != 0) ){
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                    }
                    else
                    {
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                    }
                }
            }

            else if ( i == 1 && strlen(baidu_result[1]) != 0 && ( strlen(baidu_result[2]) != 0\
                        || strlen(baidu_result[3]) != 0 || strlen(baidu_result[4]) != 0))

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);

            else if ( i == 2 && ( strlen(baidu_result[3]) != 0 || strlen(baidu_result[4]) != 0 ) )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);


            else if ( i == 3 && ( strlen(baidu_result[4]) != 0) )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
        } 

        /* 翻译结果检测为空*/
        //else if ( i == 0  && Phonetic(ONLINE)Flag == 0 && NumZhTranFlag==0
        /*&& NumEnTranFlag == 0 && OtherWordForm(ONLINE)Flag == 0) */

        /* 这里不要再用上面的用标志位检测，存在一种情况，翻译结果没复制过来，上面的结果插入语句无法执行
         * 刚好到这里的时候标志位又被刚好翻译结果写入完成的python端修改，导致重定向失败，这里的所有逻辑
         * 都不要用标志位判断*/
        else if ( i == 0  && strlen(Phonetic(ONLINE)) == 0 && strlen(ZhTrans(ONLINE))==0 \
                && strlen(EnTrans(ONLINE)) == 0 && strlen(OtherWordForm(ONLINE)) == 0){

            /* 一般来说谷歌翻译的结果获取快一点，如果百度翻译此时还没获取到，
             * 先返回谷歌翻译的结果
             *
             * 只做一次重定向，之后再按切换按钮如果还没有获取到结果显示错误信息，不然一直刷新
             * 界面都没有变化太难受了*/
            if (  WINDATA(data)->hadRedirect )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, "\n\n   获取百度翻译结果失败\n",\
                        -1, "brown-font",  "heavy-font", \
                        "font-size-11","letter-spacing", NULL);

            else if ( WINDATA(data)->hadRedirect != 1 ){

                pred("翻译结果重定向");

                WINDATA(data)->hadRedirect = 1;

                pred("flag:%c \n", shmaddr_mysql[0]);

                if ( WINDATA(data)->getOfflineTranslation )
                    return displayOfflineTrans ( WINDATA(data)->offlineButton, data);
                else
                    return displayGoogleTrans ( WINDATA(data)->googleButton, data);
            }
        }
    }
}

void setFontProperties(GtkTextBuffer *buf, GtkTextIter *iter) {

    /*注意属性值设置正确，不然桌面分分钟崩溃:(*/

    gtk_text_buffer_create_tag(buf, "black-font", "foreground", "#000000", NULL);
    gtk_text_buffer_create_tag(buf, "yellow-font", "foreground", "#c8ab02", NULL);
    gtk_text_buffer_create_tag(buf, "blue-font", "foreground", "#00aaff", NULL);
    gtk_text_buffer_create_tag(buf, "brown-font", "foreground", "#606415", NULL);
    gtk_text_buffer_create_tag(buf, "green-font", "foreground", "#216459", NULL);
    gtk_text_buffer_create_tag(buf, "light-yellow-font", "foreground", "#a3aa89", NULL);
    gtk_text_buffer_create_tag(buf, "bold-style", "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag(buf, "heavy-font", "weight", PANGO_WEIGHT_HEAVY, NULL);
    //gtk_text_buffer_create_tag(buf,  "Uneditable" ,"editable", FALSE, NULL);
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

    pcyan("(In printDebugInfo) \n");

    pcyan("Finish标志位: %c", shmaddr_baidu[0]);
    pcyan("Phonetic(ONLINE)标志位: %c", shmaddr_baidu[1]);
    pcyan("Numbers of zhTrans标志位: %c", shmaddr_baidu[2]);
    pcyan("Numbers of enTrans标志位: %c", shmaddr_baidu[3]);
    pcyan("Other forms of word标志位: %c", shmaddr_baidu[4]);
    pcyan("Numbers of audio links标志位: %c\n", shmaddr_baidu[5]);

    pcyan("Finish标志位: %c", shmaddr_mysql[0]);
    pcyan("Phonetic(ONLINE)标志位: %c", shmaddr_mysql[1]);
    pcyan("Numbers of zhTrans标志位: %c", shmaddr_mysql[2]);
    pcyan("Numbers of enTrans标志位: %c", shmaddr_mysql[3]);
    pcyan("Other forms of word标志位: %c", shmaddr_mysql[4]);
    pcyan("Numbers of audio links标志位: %c\n", shmaddr_mysql[5]);

    pcyan("离线翻译结果: %s\n", &shmaddr_mysql[ACTUALSTART]);
    pcyan("百度翻译结果: %s\n", &shmaddr_baidu[ACTUALSTART]);
    pcyan("谷歌翻译结果: %s\n", &shmaddr_google[ACTUALSTART]);
    pcyan("(out printDebugInfo) ");
}

/*当窗口大小被鼠标改变时进行窗口重绘以及自动调整switch button位置*/
void syncNormalWinForConfigEvent( GtkWidget *window, GdkEvent *event, gpointer data ) {

    gint width, height;
    static unsigned int lastwidth = 0, lastheight = 0;

    gtk_window_get_size ( (GtkWindow*)window, &width, &height );

    pyellow("当前窗口大小 width=%d height=%d", width, height);
    pyellow("上一次窗口大小 width=%d height=%d", lastwidth, lastheight);
    pbred("(syncNormalWinForConfigEvent) current indicate size: %d %d", WINDATA(data)->width,WINDATA(data)->height );

    if (WINDATA(data)->width <= width &&  WINDATA(data)->height <= height)
        WINDATA(data)->specific = 0;

    /* 窗口大小未改变不用重新调整布局,直接返回*/
    if ( lastwidth == width && lastheight == height && !WINDATA(data)->specific){
        printf("Not change, return \n");
        return;
    }

    printf("Change\n");

    pred("计算应设窗口大小: %d %d", WINDATA(data)->width,WINDATA(data)->height);
    pred("specific=%d<", WINDATA(data)->specific);

    /* 指定窗口大小时*/
    if ( WINDATA(data)->specific ) {
        pred("指定窗口大小");
        width = WINDATA(data)->width;
        height = WINDATA(data)->height;

        gtk_widget_set_size_request ( (GtkWidget*)(WINDATA(data))->layout,  width, height);
    }

    lastwidth = width;
    lastheight = height;

    syncImageSize ( (WINDATA(data))->window,width,height, data );

    gtk_window_resize ( GTK_WINDOW((WINDATA(data))->window), width, height );
    gtk_widget_set_size_request ( (GtkWidget*)(WINDATA(data))->scroll,  width, height);

    /* layout不要重设打下，否则将造成无法在缩小窗口*/
    //gtk_widget_set_size_request ( (GtkWidget*)(WINDATA(data))->layout,  width, height);

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout, \
            (WINDATA(data))->baiduButton, width-RIGHT_BORDER_OFFSET, height-BOTTOM_OFFSET );

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,\
            (WINDATA(data))->googleButton, width-RIGHT_BORDER_OFFSET*2, height-BOTTOM_OFFSET );

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,\
            (WINDATA(data))->offlineButton, width-RIGHT_BORDER_OFFSET*3, height-BOTTOM_OFFSET );

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,\
            (WINDATA(data))->indicateButton, width-(200-(RIGHT_BORDER_OFFSET*(WINDATA(data))->who)), height-INDICATE_OFFSET );

    WINDATA(data)->width = width;
    WINDATA(data)->height = height;

    WINDATA(data)->lastwidth = width;
    WINDATA(data)->lastheight = height;

    gtk_widget_queue_draw ( window );
    gtk_widget_show(window);
}


/* 设置NormalWin的窗口大小*/
void setWinSizeForNormalWin (WinData *window, char *addr, int type) {

    int maxlen = 0;
    int lines = 0;

    pmag("In setWinSizeForNormalWin");

    if ( addr == shmaddr_baidu || addr == shmaddr_mysql) {

        maxlen = getMaxLenOfBaiduTrans ( type );
        lines = getLinesOfBaiduTrans ( type );

        pmag("maxlen=%d lines=%d \n", maxlen, lines);

        double width, height;

        width = maxlen * 15.6 + 42;
        height = lines * 24 + 50;

        pmag("width=%f height=%f <In setWinSizeForNormalWin 1>", width, height);

        /*别让窗口过小*/
        if ( width < 400 ) {
            width = 400;
        }

        if ( height < 300 )
            height = 300;

        pmag("width=%f height=%f <In setWinSizeForNormalWin 1.1>", width, height);


        if ( WIDTH(WHO(addr)) > 1000 )
            width = 1000;

        if ( HEIGHT(WHO(addr)) > 900 )
            height = 900;

        /*Update the window size only when the new size is larger than older's*/
        STORAGE_WIDTH(WHO(addr), width);
        STORAGE_HEIGHT(WHO(addr), height);

        if ( WINDATA(window)->width < WIDTH(WHO(addr)) )
            WINDATA(window)->width = WIDTH(WHO(addr));

        if (WINDATA(window)->height < HEIGHT(WHO(addr)))
            WINDATA(window)->height = HEIGHT(WHO(addr));

        if ( WINDATA(window)->lastheight != WINDATA(window)->height 
                || WINDATA(window)->width != WINDATA(window)->lastwidth )
            WINDATA(window)->specific = 1;

        return;
    } 

    /* mark1*/
    else if ( addr == shmaddr_google ) 
    {
    }
}

int getMaxLenOfBaiduTrans(int type) {

    char **result = NULL;
    if ( type == ONLINE )
        result = baidu_result;
    else if ( type == OFFLINE ) 
        result = mysql_result;

    int maxlen = 0;

    for ( int i=0, len=0; i<BAIDUSIZE; i++ ) {
        if ( result[i][0] != '\0')
            len = countCharNums ( result[i] );

        if ( len > maxlen )
            maxlen = len;
    }

    if ( maxlen > 28 )
        maxlen = 28;

    return maxlen;
}

int getLinesOfBaiduTrans (int type) {

    char **result = NULL;
    if ( type == ONLINE )
        result = baidu_result;
    else if ( type == OFFLINE )
        result = mysql_result;

    int resultNum = 0;

    /* lines起始为1，因为源数据没有被插入回车符，后面计算不到，这里
     * 手动加1*/
    int lines = 1;

    char *p = NULL;

    for ( int i=0; i<BAIDUSIZE; i++ ) {
        if ( result[i][0] != '\0' ) {
            resultNum++;
            p = result[i];
            while ( *p ) {
                if ( *p++ == '\n' )
                    lines++;
            }
        }
    }

    /* 算上到时插入到显示界面的空行，最后一个结果后面不插入空行，
     * 所以减1*/
    lines = lines + resultNum - 1;

    return lines;
}
