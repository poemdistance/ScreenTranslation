/* 注意不要频繁调用gtk_widget_queue_draw(),
 * 否则越靠后的重绘指令越有可能不被成功执行*/

#include "common.h"

#define BOTTOM_OFFSET ( 45 )
#define RIGHT_BORDER_OFFSET ( 50 )
#define INDICATE_OFFSET ( 80 )

extern char *text;

extern char *shmaddr_google;
extern char *shmaddr_baidu;
extern char *shmaddr_keyboard;
extern char *shmaddr_mysql;

char *baidu_result[BAIDUSIZE] = { NULL };
char *google_result[GOOGLESIZE] = { NULL };
char *mysql_result[MYSQLSIZE] = { NULL };

/* 用于和detectMouse通信，当已经新建翻译结果显示窗口时，
 * 不再检测鼠标动作*/
int InNewWin = 0;

/* 用于切换百度和谷歌翻译结果的显示, -1和0互为取反运算*/
int show = -1;
int scrollShow = -1;

/* 鼠标动作标志位*/
extern int action;

extern char audioOnline_en[512];
extern char audioOnline_uk[512];

extern char audioOffline_en[512];
extern char audioOffline_uk[512];

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
        printf("\033[0;31m窗口聚焦请求失败(focusOurWindow) \033[0m\n");
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
    if ( 0 & (shmaddr_keyboard[1] == '1') ) {

        printf("\033[0;32m窗口检测到Control-C\033[0m\n");

        WinData *wd = WINDATA(data);

        gtk_window_present_with_time ( GTK_WINDOW(wd->window), time(NULL) );
        gtk_window_set_focus ( (GtkWindow*)wd->window, wd->view );
        gtk_window_set_focus ( (GtkWindow*)wd->window, wd->scroll );
        printf("\033[0;32mfocus test>>>>%d<<< \033[0m\n", gtk_window_activate_focus ( GTK_WINDOW(wd->window) ));
        destroyNormalWin(wd->window, wd);
        return FALSE;
    }

    return TRUE;
}

void syncVolumeBtn ( WinData *wd, int type ) {

    /* 含音标，添加播放按钮*/
    if ( strlen ( Phonetic(type) ) != 0) {

        GtkWidget *volume =  ((WinData*)wd)->volume;

        if ( volume == NULL ) {

            printf("\033[0;34m插入音频播放按钮 \033[0m\n");
            bw.audio_online[0] = audio_en(ONLINE);
            bw.audio_online[1] = audio_uk(ONLINE);

            bw.audio_offline[0] = audio_en(OFFLINE);
            bw.audio_offline[1] = audio_uk(OFFLINE);

            ((WinData*)wd)->volume = insertVolumeIcon(((WinData*)wd)->window, ((WinData*)wd)->layout, ((WinData*)wd), type);
        }
        else {

            printf("\033[0;34m显示音频播放按钮\033[0m\n");
            gtk_widget_show ( volume );
        }

    }

}

/* 程序会在oh前面崩溃*/
void mark_set(GtkTextBuffer *buf, gpointer *data) {

    WinData *wd = WINDATA(data);

    GtkTextIter start ;
    GdkRectangle loc;
    printf("god\n");
    //gtk_text_buffer_get_iter_at_mark ( buf, &start, gtk_text_buffer_get_mark ( buf, "GOD" ) );
    gtk_text_buffer_get_start_iter ( buf, &start );

    printf("please\n");
    gtk_text_view_get_iter_location ( (GtkTextView*)(wd->view), &start, &loc );
    printf("oh\n");
    (WINDATA(data))->lineHeight = loc.y;

    printf("\033[0;31mloc.x=%d loc.y=%d \033[0m\n", loc.x, loc.y);

    printf("\033[0;31m行高=%d \033[0m\n", loc.y);
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

    printf("\033[0;31mloc.x=%d loc.y=%d \033[0m\n", loc.x, loc.y);

    printf("\033[0;31m行高=%d \033[0m\n", loc.y);
}


/*新建翻译结果窗口, 本文件入口函数*/
void *newNormalWindow() {

    /* Storage the relative element or data in this window*/
    WinData wd;

    focustimes = 1;
    show = -1;
    InNewWin = 1;

    printf("\n准备判断是否新建一般窗口\n\n");

    /* 窗口打开标志位 changed in captureShortcutEvent.c <变量shmaddr>*/
    shmaddr_keyboard[2] = '1';

    wd.getOfflineTranslation = 0;

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
    if ( shmaddr_keyboard[0] == '1') {
        shmaddr_keyboard[0] = '0';

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

    //gtk_layout_put(GTK_LAYOUT(layout), scroll, 0, 0);

    //printDebugInfo();
    setFontProperties(buf, &iter);

    wd.image =  NULL;
    wd.oldImage = NULL;
    wd.view = view;
    wd.window = newWin;
    wd.layout = layout;
    wd.srcBackgroundImage = NULL;
    wd.gdkwin = NULL;
    wd.width = 0;
    wd.height = 0;
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
    wd.image = syncImageSize ( newWin, (void*)&wd );

    int index_baidu[13] = { 0 };
    int index_mysql[13] = { 0 };

    printDebugInfo();

    //TODO:if ( shmaddr_baidu[0] !=ERRCHAR )
    getIndex(index_baidu, shmaddr_baidu);
    getIndex(index_mysql, shmaddr_mysql);

    printf("\033[0;36m百度翻译分隔符索引结果:\033[0m ");
    for (int i=0; i<13; i++)
        printf("\033[0;33m%d \033[0m ", index_baidu[i]);
    printf("\n");

    /*初始化百度以及离线翻译结果存储空间*/
    initMemoryBaidu();
    initMemoryMysql();

    /* 从共享内存数据流中分离相关数据到baidu_result相关功能内存区域,
     * 第二个参数是单行显示字符长度*/
    separateDataForBaidu(index_baidu, 28, ONLINE);
    separateDataForBaidu(index_mysql, 28, OFFLINE);

    int maxlen_baidu  = getMaxLenOfBaiduTrans ( ONLINE );
    int lines_baidu =  getLinesOfBaiduTrans ( ONLINE );

    int maxlen_mysql  = getMaxLenOfBaiduTrans ( OFFLINE );
    int lines_mysql =  getLinesOfBaiduTrans ( OFFLINE );

    /*根据翻译结果的最大长度和行数设置窗口大小, 
     *The result of below two statements are equivalent in normal condition,
     *but the first one might not get the proper size of window because of 
     *the failure of getting translation data from baidu.translate.com sometimes*/
    setWinSizeForNormalWin ( maxlen_baidu, lines_baidu, shmaddr_baidu, ONLINE );
    setWinSizeForNormalWin ( maxlen_mysql, lines_mysql, shmaddr_mysql, OFFLINE );

    printf("\033[0;33m(NewWin) baidu_maxlen=%d lines_baidu=%d \n\033[0m",maxlen_baidu, lines_baidu);


    /*根据前面得出的宽高设置窗口和scrolled window的宽高值*/
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

    //g_signal_connect ( buf, "mark-set", G_CALLBACK(mark_set), (void*)&wd );

    /*显示百度翻译结果*/
    displayBaiduTrans(baiduButton, (void*)&wd );

    gtk_main();

    pthread_exit(NULL);
}

void clearMemory () {

    /* 标志位空间用字符0填充*/
    memset(shmaddr_baidu, '0', 10);
    memset(shmaddr_mysql, '0', 10);

    memset(shmaddr_google, '\0', SHMSIZE);
    memset(shmaddr_baidu, '\0', SHMSIZE-10);
    memset(shmaddr_mysql, '\0', SHMSIZE-10);

    memset ( audioOnline_uk, '\0', 512 );
    memset ( audioOnline_en, '\0', 512 );

    for ( int i=0; i<BAIDUSIZE; i++ )
        if ( baidu_result[i] != NULL)
            memset( baidu_result[i], '\0', SHMSIZE / BAIDUSIZE );

    for ( int i=0; i<MYSQLSIZE; i++ )
        if ( mysql_result[i] != NULL)
            memset( mysql_result[i], '\0', SHMSIZE / MYSQLSIZE );

    for ( int i=0; i<GOOGLESIZE; i++ )
        if ( google_result[i] != NULL)
            memset( google_result[i], '\0', SHMSIZE / GOOGLESIZE );
}

int destroyNormalWin(GtkWidget *window, WinData *wd) {

    clearMemory();

    g_source_remove ( timeout_id );

    /* 窗口关闭标志位*/
    shmaddr_keyboard[2] = '0';
    shmaddr_keyboard[1] = '0';

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
                printf("\033[0;33m(Google)已完成分割字符索引 \033[0m\n");
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

    printf("\033[0;32mCanNewWin=%d \033[0m\n", CanNewWin);

    /*等待任意一方python端的翻译数据全部写入共享内存*/
    while(/* (CanNewWin != 1) || */ ( shmaddr_google[0] != FINFLAG && shmaddr_baidu[0] != FINFLAG \
                && shmaddr_mysql[0] != FINFLAG )) {

        printf("\033[0;32mI an coming here \033[0m\n");

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
            printf("\033[0;31m超时退出 \033[0m\n");
            shmaddr_google[0] = ERRCHAR;
            shmaddr_baidu[0] = ERRCHAR;
            break;
        }

    }

    printf("\033[0;32m准备创建窗口 %p \033[0m\n", text);

    if ( text == NULL )
        if (( text = calloc(TEXTSIZE, 1)) == NULL)
            err_exit("malloc failed in notify.c");

    if ( shmaddr_baidu[0] == EXITFLAG ) {
        fprintf(stderr, "\033[0;35m百度翻译异常退出...\n\033[0m");
        fprintf(stderr, "\033[0;35m正在停止取词翻译...\n\033[0m");
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

/* 初始化离线翻译结果存储空间*/
void initMemoryMysql() {

    if (mysql_result[0] != NULL)
        return;

    for (int i=0; i<MYSQLSIZE; i++) {

        mysql_result[i] = calloc(SHMSIZE / MYSQLSIZE, sizeof ( char ) );
        if (mysql_result[i] == NULL)
            err_exit("Error occured when calloc memory in initMemoryMysql");
    }
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
void reGetBaiduTransAndSetWin (gpointer *data, int which ) { 

    printf("\033[0;36mIn reGetBaiduTrans \033[0m\n");
    printf("\033[0;36m重新从共享内存获取百度翻译数据 \033[0m\n");

    int index[13] = { 0 };

    //printDebugInfo();

    getIndex(index, shmaddr_baidu );

    separateDataForBaidu(index, 28, ONLINE );

    int maxlen= getMaxLenOfBaiduTrans(ONLINE);
    int lines= getLinesOfBaiduTrans(ONLINE);

    setWinSizeForNormalWin ( maxlen, lines, shmaddr_baidu, ONLINE);

    printf("\033[0;36m(reGetBaiduTrans)maxlen=%d lines_baidu=%d bw.width=%f bw.height=%f\033[0m\n", maxlen, lines, bw.width, bw.height);

}

void adjustWinSize(GtkWidget *button, gpointer *data, int which ) {

    if ( !which ) 
    {
        int index[2] = { 0 };

        printf("\033[0;31m谷歌翻译重新索引 index: \033[0m");
        getIndex(index, shmaddr_google);

        /* 找到分割符，数据分离提取才有意义*/
        if ( index[0] != 0 )
            separateGoogleDataSetWinSize ( index );

        /*如果新窗口的宽高都小于上一个的，不调整窗口大小*/
        if ( gw.width <= bw.width && gw.height <= bw.height) {

            /* 原谷歌scrolled window小于百度的，窗口没变，但scrolled window要随之扩大*/
            gtk_window_resize ( (GtkWindow*)WINDATA(data)->window, bw.width, bw.height );
            gtk_widget_set_size_request ( WINDATA(data)->scroll, bw.width, bw.height );
            //gtk_layout_move((GtkLayout*)WINDATA(data)->layout, button, bw.width-RIGHT_BORDER_OFFSET, bw.height-BOTTOM_OFFSET);
            //gtk_widget_queue_draw ( WINDATA(data)->window );
            gtk_widget_show_all(WINDATA(data)->window);
            return;
        }

        if (gw.width < 400)
            gw.width = 400;

        if ( gw.height < 230 )
            gw.height = 230;

        if ( gw.width > 1000 )
            gw.width = 1000;

        if ( gw.height > 900 )
            gw.height = 900;

        WINDATA(data)->width = gw.width;
        WINDATA(data)->height = gw.height;

        /* 以下代码段会触发configure-event事件而调用对应回调函数: syncNormalWinForConfiEvent*/
        gtk_window_resize((GtkWindow*)WINDATA(data)->window, gw.width, gw.height);
        gtk_widget_set_size_request ( WINDATA(data)->scroll, gw.width, gw.height );
        //gtk_layout_move((GtkLayout*)WINDATA(data)->layout, button, gw.width-RIGHT_BORDER_OFFSET, gw.height-BOTTOM_OFFSET);
        //gtk_widget_queue_draw ( WINDATA(data)->window );
        gtk_widget_show_all(WINDATA(data)->window);
        printf("\033[0;31m\n谷歌翻译重设窗口大小:gw width=%f gw.height=%f\033[0m\n", gw.width, gw.height);
    } 

    else 
    {
        /*还未获取到结果，应重新获取并设置窗口大小*/
        if ( strlen ( ZhTrans(ONLINE) ) == 0) {

            printf("\033[0;36m百度翻译结果长度为0\033[0m\n");
            reGetBaiduTransAndSetWin ( data, which );
        }

        /*如果新窗口的宽高都小于上一个的，不调整窗口大小*/
        if ( gw.width >= bw.width && gw.height >= bw.height) {

            printf("\033[0;34m百度翻译窗口小于谷歌，可不必调整 \033[0m\n");
            gtk_window_resize ( (GtkWindow*)WINDATA(data)->window, gw.width, gw.height );
            gtk_widget_set_size_request ( WINDATA(data)->scroll, gw.width, gw.height );
            //gtk_layout_move((GtkLayout*)WINDATA(data)->layout, button, gw.width-RIGHT_BORDER_OFFSET, gw.height-BOTTOM_OFFSET);
            //gtk_widget_queue_draw ( WINDATA(data)->window );
            gtk_widget_show_all(WINDATA(data)->window);
            return;
        }

        if ( bw.width < 400 )
            bw.width = 400;

        if ( bw.height <= 0 )
            bw.height = bw.width * 0.618;

        if ( bw.width > 1000 )
            bw.width = 1000;

        if ( bw.height > 900 )
            bw.height = 900;

        WINDATA(data)->width = bw.width;
        WINDATA(data)->height = bw.height;

        printf("\033[0;35mbw width=%f %f\033[0m\n", bw.width, bw.height);
        printf("\033[0;35m百度翻译重设窗口大小 \033[0m\n");
        gtk_window_resize((GtkWindow*)WINDATA(data)->window, bw.width, bw.height);
        gtk_widget_set_size_request ( WINDATA(data)->scroll, bw.width, bw.height );
        //gtk_layout_move((GtkLayout*)WINDATA(data)->layout, button, bw.width-RIGHT_BORDER_OFFSET, bw.height-BOTTOM_OFFSET);
        //gtk_widget_queue_draw ( WINDATA(data)->window );
        gtk_widget_show_all(WINDATA(data)->window);
    }
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

    gtk_layout_move ( (GtkLayout*)WINDATA(data)->layout,\
            WINDATA(data)->indicateButton, WINDATA(data)->width-RIGHT_BORDER_OFFSET*2, WINDATA(data)->height-INDICATE_OFFSET );

    gtk_widget_queue_draw ( WINDATA(data)->window );

    printf("\033[0;33m\n显示谷歌翻译结果:\033[0m\n\n");

    /* 调整窗口大小*/
    adjustWinSize ( button, data, 0 );

    if ( WINDATA(data)->volume != NULL ) {
        printf("\033[0;31m隐藏音频按钮 \033[0m\n");
        gtk_widget_hide ( WINDATA(data)->volume  );
    }

    GtkTextIter *iter, start, end;
    iter = WINDATA(data)->iter;
    GtkTextBuffer *buf = WINDATA(data)->buf;

    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_get_end_iter(buf, &end);

    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);

    /* TODO:*/
    WINDATA(data)->iter = iter ;

    int index_google[2] = { 0 };

    /* 比较字符串是否相等,如果不相等，说明用于谷歌翻译结果存储的共享内存被改写了，
     * 需要重新分离调整字符串*/
    if ( strcmp ( &shmaddr_google[ACTUALSTART], google_result[0] ) != 0 ) {

        printf("\033[0;31m字符串不相等: google_result[0]->%s< \033[0m\n\n", google_result[0]);
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
        printf("\033[0;33m字符串依然相等\033[0m\n");

        /*若字符串依旧相等，直接拿来用就行*/
    }

    printf("\033[0;31mCall syncImageSize in disGoogle \033[0m\n");

    /* mark*/
    syncImageSize ( WINDATA(data)->window, data) ;

    char enter[] = "\n";

    /*插入输入原文*/
    if ( strlen( text )  < 30 ) {

        gtk_text_buffer_insert_with_tags_by_name(buf, iter, text, -1, 
                "black-font",  "bold-style",  "font-size-15", "underline", NULL);

        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
        //gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
    }

    printf("\033[0;33mdisplayGoogleTrans: \033[0m\n");
    /*插入翻译结果*/
    for ( int i=0; i<3; i++ ) {

        //printf("\033[0;33mgoogle_result[%d]=%s\033[0m\n", i, google_result[i]);
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

    WINDATA(data)->who = OFFLINE;

    gtk_layout_move ( (GtkLayout*)WINDATA(data)->layout,\
            WINDATA(data)->indicateButton, WINDATA(data)->width-RIGHT_BORDER_OFFSET*3, WINDATA(data)->height-INDICATE_OFFSET );

    gtk_widget_queue_draw ( WINDATA(data)->window );

    printf("\033[0;36mDisplaying offline translation \033[0m\n");

    GtkTextIter *iter, start, end;
    iter = WINDATA(data)->iter;
    GtkTextBuffer *buf = WINDATA(data)->buf;

    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_get_end_iter(buf, &end);

    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);

    /* ? TODO*/
    WINDATA(data)->iter = iter ;
    syncImageSize ( WINDATA(data)->window, data) ;

    if ( ! WINDATA(data)->getOfflineTranslation )
        gtk_text_buffer_insert_with_tags_by_name(buf, iter, "\n  NOT FOUND ANYTHING",\
                -1, "yellow-font",  "heavy-font", \
                "font-size-11", "letter-spacing", NULL);


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
                printf("create mark\n");
                gtk_text_buffer_create_mark ( buf, "GOD", &end, TRUE);

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, mysql_result[i],\
                        -1, "blue-font",  "heavy-font", \
                        "font-size-11", "letter-spacing", NULL);

                syncVolumeBtn ( WINDATA(data), ONLINE );
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


    adjustWinSize ( button, data, 1 );

    WINDATA(data)->who = BAIDU;

    gtk_layout_move ( (GtkLayout*)WINDATA(data)->layout,\
            WINDATA(data)->indicateButton, WINDATA(data)->width-RIGHT_BORDER_OFFSET, WINDATA(data)->height-INDICATE_OFFSET );

    gtk_widget_queue_draw ( WINDATA(data)->window );


    GtkTextBuffer *buf = WINDATA(data)->buf;

    printf("\033[0;36m显示百度翻译结果\033[0m\n");

    if ( strlen (ZhTrans(ONLINE)) == 0 && strlen ( EnTrans(ONLINE) ) == 0 ) {

        printf("\033[0;36mZhTrans(ONLINE) & EnTrans(ONLINE)长度皆为0 \033[0m\n");

        reGetBaiduTransAndSetWin ( data, -1 );
    }

    if ( strcmp ( &shmaddr_baidu[ACTUALSTART], baidu_result[0] ) != 0) {

        printf("\033[0;31m字符串不相等，接收到新的百度翻译，重新获取 \033[0m\n");
        printf("\033[0;35m>%s< \033[0m\n",&shmaddr_baidu[ACTUALSTART]);
        printf("\033[0;35m>%s< \033[0m\n",baidu_result[0]);
        reGetBaiduTransAndSetWin ( data, -1 );
    }

    GtkTextIter start, end, *iter;
    iter = WINDATA(data)->iter;

    /*找到开头和结尾并删除，重新定位到初始为位置0*/
    gtk_text_buffer_get_end_iter(buf, &end);
    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);

    WINDATA(data)->iter = iter ;

    /* mark*/
    syncImageSize ( WINDATA(data)->window, data) ;

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
                printf("create mark\n");
                gtk_text_buffer_create_mark ( buf, "GOD", &end, TRUE);

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i],\
                        -1, "blue-font",  "heavy-font", \
                        "font-size-11", "letter-spacing", NULL);

                syncVolumeBtn ( WINDATA(data) , ONLINE);
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

                printf("\033\n\n[0;31m 翻译结果重定向 \033[0m\n\n");

                WINDATA(data)->hadRedirect = 1;

                printf("\033[0;31mflag:%c \033[0m\n", shmaddr_mysql[0]);

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

    printf("\033[0;35m(In printDebugInfo) \033[0m\n");

    printf("\n\033[0;36mFinish标志位: %c\033[0m", shmaddr_baidu[0]);
    printf("\n\033[0;36mPhonetic(ONLINE)标志位: %c\033[0m", shmaddr_baidu[1]);
    printf("\n\033[0;36mNumbers of zhTrans标志位: %c\033[0m", shmaddr_baidu[2]);
    printf("\n\033[0;36mNumbers of enTrans标志位: %c\033[0m", shmaddr_baidu[3]);
    printf("\n\033[0;36mOther forms of word标志位: %c\033[0m", shmaddr_baidu[4]);
    printf("\n\033[0;36mNumbers of audio links标志位: %c\033[0m\n", shmaddr_baidu[5]);

    printf("\n\n\033[0;36mFinish标志位: %c\033[0m", shmaddr_mysql[0]);
    printf("\n\033[0;36mPhonetic(ONLINE)标志位: %c\033[0m", shmaddr_mysql[1]);
    printf("\n\033[0;36mNumbers of zhTrans标志位: %c\033[0m", shmaddr_mysql[2]);
    printf("\n\033[0;36mNumbers of enTrans标志位: %c\033[0m", shmaddr_mysql[3]);
    printf("\n\033[0;36mOther forms of word标志位: %c\033[0m", shmaddr_mysql[4]);
    printf("\n\033[0;36mNumbers of audio links标志位: %c\033[0m\n\n", shmaddr_mysql[5]);

    printf("\033[0;34m离线翻译结果: %s \033[0m\n", &shmaddr_mysql[ACTUALSTART]);
    printf("\n\033[0;36m百度翻译结果: %s\n\n\033[0m", &shmaddr_baidu[ACTUALSTART]);
    printf("\033[0;33m谷歌翻译结果: %s\033[0m\n\n", &shmaddr_google[ACTUALSTART]);
    printf("\033[0;35m(out printDebugInfo) \033[0m\n");
}

/*当窗口大小被鼠标改变时进行窗口重绘以及自动调整switch button位置*/
void syncNormalWinForConfigEvent( GtkWidget *window, GdkEvent *event, gpointer data ) {

    //printf("\033[0;35m进入窗口控件同步函数 \033[0m\n");

    gint width, height;
    static unsigned int lastwidth = 0, lastheight = 0;

    gtk_window_get_size ( (GtkWindow*)window, &width, &height );

    if ( (WINDATA(data))->width > width ||  (WINDATA(data))->height > height ) {

        width  = (WINDATA(data))->width ;
        height  = (WINDATA(data))->height ;
    }

    //printf("\033[0;35m当前窗口大小 width=%d height=%d \033[0m\n", width, height);
    //printf("\033[0;35m上一次窗口大小 width=%d height=%d \033[0m\n", lastwidth, lastheight);

    /* 窗口大小未改变不用重新调整布局,直接返回*/
    if ( lastwidth == width && lastheight == height )
        return;

    lastwidth = width;
    lastheight = height;

    //printf("\033[0;35m同步普通窗口相关控件 \033[0m\n");

    gtk_window_resize ( GTK_WINDOW((WINDATA(data))->window), width, height );
    gtk_widget_set_size_request ( (GtkWidget*)(WINDATA(data))->scroll,  width, height);

    /* Unnecessary*/
    //gtk_widget_queue_draw ( (WINDATA(data))->baiduButton );
    //gtk_widget_show (  (WINDATA(data))->baiduButton  );

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout, \
            (WINDATA(data))->baiduButton, width-RIGHT_BORDER_OFFSET, height-BOTTOM_OFFSET );

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,\
            (WINDATA(data))->googleButton, width-RIGHT_BORDER_OFFSET*2, height-BOTTOM_OFFSET );

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,\
            (WINDATA(data))->offlineButton, width-RIGHT_BORDER_OFFSET*3, height-BOTTOM_OFFSET );

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,\
            (WINDATA(data))->indicateButton, width-(RIGHT_BORDER_OFFSET*(WINDATA(data))->who), height-INDICATE_OFFSET );

    /* mark*/
    syncImageSize ( (WINDATA(data))->window, data );

    gtk_widget_queue_draw ( window );
}

/*跟上一个函数是一样的, 这里主要是考虑到后期拓展和区分窗口写成了两个函数*/
void syncScrolledWinWithConfigEvent ( GtkWidget *window, GdkEvent *event, gpointer *wd ) {

    gint width, height;
    static unsigned int lastwidth = 0, lastheight = 0;

    //printf("syncScrolledWinWithConfigEvent\n");
    gtk_window_get_size ( (GtkWindow*)window, &width, &height );

    /* 窗口大小未改变不用重新调整布局,直接返回*/
    if ( lastwidth == width && lastheight == height )
        return;

    lastwidth = width;
    lastheight = height;

    gtk_window_resize ( GTK_WINDOW(((WinData*)wd)->window), width, height );
    gtk_widget_set_size_request ( (GtkWidget*)((WinData*)wd)->scroll,  width, height);
    gtk_layout_move ( (GtkLayout*)((WinData*)wd)->layout, ((WinData*)wd)->baiduButton, width-RIGHT_BORDER_OFFSET, height-BOTTOM_OFFSET );

    syncImageSize ( ((WinData*)wd)->window, (void*)wd );
    ((WinData*)wd)->width = width;
    ((WinData*)wd)->height = height;

    gtk_widget_queue_draw ( window );
    gtk_widget_show_all ( ((WinData*)wd)->window );
}

void resizeScrolledWin ( WinData *wd, gint width, gint height ) {

    //wd->forceResize = 1;
    wd->height = height;
    wd->width = width;
    syncImageSize ( wd->window, (void*)wd );

    printf("\033[0;33mresize scrolled window %d %d \033[0m\n", width, height);
    gtk_window_resize ((GtkWindow*) wd->window, width, height );
    gtk_widget_set_size_request ( wd->scroll,  width, height);

    gtk_layout_move ( (GtkLayout*)wd->layout, wd->baiduButton, width-RIGHT_BORDER_OFFSET, height-BOTTOM_OFFSET );
    gtk_widget_queue_draw ( wd->window );

    gtk_widget_show_all(wd->window);

    gtk_window_get_size ( (GtkWindow*)(wd->window), &width, &height );
    printf("\033[0;33mnow size %d %d \033[0m\n", width, height);

}

/* Suit the size of window with the number of characters */
void suitWinSizeWithCharNum ( char *addr , WinData *wd) {

    if ( addr == shmaddr_baidu ) {

        int charNums = countCharNums ( &shmaddr_baidu[ACTUALSTART+2] );

        printf("\033[0;35m (showBaiduScrolledWin) charNums=%d\n", charNums);

        /* 根据字符数量控制窗口大小和单行长度*/
        if ( charNums < 400 ) {

            int lines = countLines ( 30 , &shmaddr_baidu[ACTUALSTART+2] );
            printf("\033[0;35m (showBaiduScrolledWin) lines=%d\n", lines);
            adjustStrForScrolledWin ( 30, &shmaddr_baidu[2+ACTUALSTART] );
            wd->width = 650;
            wd->height = lines * 30 + 45;

            if ( wd->height < 400 )
                wd->height = 400;

            if ( wd->height > 600 )
                wd->height = 600;
        } 
        else  {

            int lines = countLines ( 30 , &shmaddr_baidu[ACTUALSTART+2] );
            printf("\033[0;35mbaiduRestlt countLines=%d \033[0m\n", lines);
            wd->width = 950;
            wd->height = lines * 30;

            if ( wd->height < 400 )
                wd->height = 400;

            if ( wd->height > 600 )
                wd->height = 600;

            adjustStrForScrolledWin ( 46, &shmaddr_baidu[ACTUALSTART+2] );
            strcpy ( ZhTrans(ONLINE),  &shmaddr_baidu[ACTUALSTART+2]);
        }
    }

    if ( addr == shmaddr_google ) {

        int index[2] = { 0 };
        getIndex(index, shmaddr_google);

        int charNums = countCharNums ( &shmaddr_google[ACTUALSTART] );
        printf("\033[0;33mcharNums=%d \033[0m\n", charNums);

        /* 根据字符数量控制窗口大小和单行长度*/
        if ( charNums < 400 ) {

            int lines = countLines ( 30 , &shmaddr_google[ACTUALSTART] );
            adjustStrForScrolledWin( 30, &shmaddr_google[ACTUALSTART] );
            wd->width = 650;
            wd->height = lines * 30;
            if ( wd->height < 500 )
                wd->height = 400;

            if ( wd->height > 600 )
                wd->height = 600;
        }
        else {

            int lines = countLines ( 30 , &shmaddr_google[ACTUALSTART] );
            wd->width = 950;
            wd->height = lines * 30;
            if ( wd->height < 400 )
                wd->height = 400;

            if ( wd->height > 600 )
                wd->height = 600;

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

    printf("\033[0;36m (showBaiduScrolledWin) width=%d height=%d\n", wd->width, wd->height);

    /* getIndex 会将分隔符修改为'\0', 再一次进入这个函数index[1]会成0
     * 所以这里是为了区分是否是第一次进入这里，如果不是，插入已经保存
     * 到ZhTrans(ONLINE)的数据, 否则先生成ZhTrans(ONLINE)*/
    if ( index[1] != 0 ) {

        shmaddr_baidu[0] = '0';

        suitWinSizeWithCharNum ( shmaddr_baidu , wd );

        resizeScrolledWin ( wd, wd->width, wd->height );
        printf("\033[0;36m (showBaiduScrolledWin) width=%d height=%d\n", wd->width, wd->height);

        strcpy ( ZhTrans(ONLINE),  &shmaddr_baidu[2+ACTUALSTART]);

        printf("\033[0;31m 调整后输出的百度翻译结果:%s\033[0m\n", ZhTrans(ONLINE));

        gtk_text_buffer_insert_with_tags_by_name ( gtbuf, iter, ZhTrans(ONLINE), -1,\
                "brown-font", "font-size-14", "bold-style", NULL );
    }

    else if ( strlen ( ZhTrans(ONLINE) ) != 0 ) {

        //adjustStrForScrolledWin ( 30, &shmaddr_baidu[ACTUALSTART+2] );
        //strcpy ( ZhTrans(ONLINE),  &shmaddr_baidu[ACTUALSTART+2]);

        /* 窗口大了，对显示的字符串进行相应调整, 扩大单行显示长度为46*/
        if ( wd->width >= 900 ) {

            adjustStrForScrolledWin ( 46, &shmaddr_baidu[ACTUALSTART+2] );
            strcpy ( ZhTrans(ONLINE),  &shmaddr_baidu[ACTUALSTART+2]);
        }

        resizeScrolledWin ( wd, wd->width, wd->height );

        printf("\033[0;36m ZhTrans(ONLINE)里的结果输出\033[0m\n");

        gtk_text_buffer_insert_with_tags_by_name ( gtbuf, iter, ZhTrans(ONLINE), -1,\
                "brown-font", "font-size-14", "bold-style", NULL );
    }
    else {

        printf("\033[0;31m 未获取到百度翻译结果\033[0m\n");

        gtk_text_buffer_insert_with_tags_by_name ( gtbuf, iter, "未获取到百度翻译结果\n", -1,\
                "green-font", "font-size-14", "bold-style", NULL );

        /* 重定向到谷歌翻译结果*/
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

        printf("\033[0;33mwindow width,height %d %d \033[0m\n", wd->width, wd->height);

        resizeScrolledWin ( wd, wd->width, wd->height );

        wd->hadShowGoogleResult  = 1;
    }

    gtk_text_buffer_insert_with_tags_by_name ( gtbuf, iter, &shmaddr_google[ACTUALSTART], -1,\
            "brown-font", "font-size-14", "bold-style", NULL );
}

/* 切换百度谷歌翻译结果显示*/
void switchScrolledWin(GtkWidget *button, gpointer data) {

    scrollShow= ~scrollShow;

    if ( scrollShow ) {
        showBaiduScrolledWin((WINDATA(data))->buf, (WINDATA(data))->iter, WINDATA(data));
        return;
    }

    showGoogleScrolledWin((WINDATA(data))->buf, (WINDATA(data))->iter, WINDATA(data));
}

int  newScrolledWin() {

    GtkWidget *window;

    /* 进入窗口标志变量，禁止继续检测鼠标状态*/
    InNewWin = 1;

    gint width = 950;
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
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    gtk_window_set_default_size (GTK_WINDOW(window), width, height);
    gtk_window_set_title (GTK_WINDOW(window), "STRAN");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);

    GtkWidget *layout = gtk_layout_new (NULL, NULL);
    GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);
    GtkWidget *view = gtk_text_view_new();

    gtk_text_view_set_editable ( (GtkTextView*)view, FALSE );
    gtk_text_view_set_cursor_visible ( ( GtkTextView* )view, FALSE );

    /* window->layout->scroll->view*/
    gtk_container_add ( GTK_CONTAINER(scroll), view );
    gtk_container_add ( GTK_CONTAINER(layout), scroll );
    gtk_container_add (GTK_CONTAINER(window), layout);

    gtk_widget_set_size_request ( scroll,  width, height);

    GtkTextBuffer *gtb = gtk_text_view_get_buffer((GtkTextView*)view);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (view), 25);
    gtk_text_view_set_top_margin ( (GtkTextView*)view, 35 );

    WinData wd;

    GtkWidget *button = newBaiduButton( (WinData*)&wd );

    gtk_layout_put ( (GtkLayout*)layout, button, width-RIGHT_BORDER_OFFSET, height-BOTTOM_OFFSET );

    GtkTextIter iter;

    wd.layout = layout;
    wd.baiduButton = button;
    wd.scroll = scroll;
    wd.buf = gtb;
    wd.width = width;
    wd.height = height;
    wd.iter = &iter;
    wd.window = window;
    wd.hadShowGoogleResult = 0;

    printf("In syncImageSize\n");
    wd.oldImage = NULL;
    wd.srcBackgroundImage = NULL;
    wd.image = syncImageSize ( window, (void*)&wd );
    printf("Out\n");

    g_signal_connect (window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect (window, "configure-event", G_CALLBACK(syncScrolledWinWithConfigEvent), &wd);
    g_signal_connect (button, "clicked", G_CALLBACK(switchScrolledWin), &wd);

    setFontProperties (gtb, &iter);

    scrollShow = -1;

    /*初始化百度翻译结果存储空间内存*/
    if ( baidu_result[0] == NULL)
        initMemoryBaidu();

    if ( google_result[0] == NULL)
        initMemoryGoogle();

    showBaiduScrolledWin(gtb, &iter, &wd);

    gtk_widget_show_all(window);

    gtk_main();

    InNewWin = 0;

    /* 清除共享内存和结果存储空间的数据*/
    clearMemory();

    pthread_exit(NULL);

    /* 窗口关闭标志位*/
    shmaddr_keyboard [2] = '0';

    return 1;
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

/* 设置NormalWin的窗口大小*/
void setWinSizeForNormalWin ( int maxlen, int lines, char *addr, int type) {

    maxlen = 0;
    lines = 0;

    printf("\033[0;35mIn setWinSizeForNormalWin \033[0m\n");

    if ( addr == shmaddr_baidu || addr == shmaddr_mysql) {

        maxlen = getMaxLenOfBaiduTrans ( type );
        lines = getLinesOfBaiduTrans ( type );

        printf("\033[0;36mmaxlen=%d lines=%d \033[0m\n", maxlen, lines);

        /*来个黄金比例的矩形*/
        double width, height;

        width = maxlen * 15.6 + 40;
        height = lines * 24 + 45;

        /*别让窗口过小*/
        if ( width < 400 ) {
            width = 400;
        }

        if ( height < 200 )
            height = 200;

        /*Update the window size only when the new size is ldataer than older's*/
        if ( width > bw.width && height > bw.height ) {

            bw.width = width;
            bw.height = height;
        }

        if ( bw.width > 1000 )
            bw.width = 1000;

        if ( bw.height > 900 )
            bw.height = 900;

        printf("\033[0;36mbw.width=%f bw.height=%f \033[0m\n", width, height);

        return;
    } 

    /* mark1*/
    else if ( addr == shmaddr_google ) 
    {
    }
}
