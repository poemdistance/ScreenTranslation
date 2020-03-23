#include <gtk/gtk.h>

/* 注意不要频繁调用gtk_widget_queue_draw(),
 * 否则越靠后的重绘指令越有可能不被成功执行*/

#include "common.h"
#include "newWindow.h"
#include "audio.h"
#include "cleanup.h"
#include "fitting.h"
#include "expanduser.h"
#include "dataStatistics.h"
#include "memoryControl.h"
#include "windowData.h"
#include "pointer.h"
#include "configControl.h"

typedef void (*Display_func)(GtkWidget *, gpointer *);

char **baidu_result[BAIDUSIZE] = { NULL };
char *google_result[GOOGLESIZE] = { NULL };
char **mysql_result[MYSQLSIZE] = { NULL };
char *tmp;

/* 用于和detectMouse通信，当已经新建翻译结果显示窗口时，
 * 不再检测鼠标动作*/
int InNewWin = 0;

/* 鼠标动作标志位*/
extern int action;
extern int mouseNotRelease;
extern int CanNewWin;
static int focustimes = 0;
int timeout_id = 0;
int movewindow_timeout_id = 0;
static int moveDone = 1;

static inline int previousWindow ( int who ) {
    return who > 1 ? who -1 : 3;
}

static inline Display_func choice_display( int who ) {

    return ( who == BAIDU ? displayBaiduTrans : \
            ( who == GOOGLE ? displayGoogleTrans :\
              displayOfflineTrans) );
}

int checkWindowSize ( gpointer *data ) {

    gint cwidth = 0;
    gint cheight = 0;
    gint iwidth = WINDATA(data)->width;
    gint iheight = WINDATA(data)->height;
    gtk_window_get_size((GtkWindow*)WINDATA(data)->window, &cwidth, &cheight);

    pbred ( "Indicate window size ?= Current window size, %d - %d   %d - %d" , iwidth, iheight, cwidth, cheight);

    if ( iwidth != cwidth || iheight != cheight ) {
        pbred ( "Indicate window size != Current window size,\
                %d - %d   %d - %d" , iwidth, iheight, cwidth, cheight);
        choice_display(WINDATA(data)->who) ( GET_BUTTON (data, WINDATA(data)->who), (void*)data );
    }

    return 0;
}

void selectDisplay( WinData *wd ) {

    if ( wd->gotBaiduTran )
        wd->who = BAIDU;
    else if ( wd->gotOfflineTran )
        wd->who = MYSQL;
    else
        wd->who = GOOGLE;

    choice_display ( wd->who )(GET_BUTTON ( wd, wd->who ), (void*)wd);
}

int adjustTargetPosition( 
        int *targetx, int *targety,
        int pointerx, int pointery,
        WinData *wd ) {

    ConfigData *cd = wd->cd;
    GdkRectangle workarea = { '\0' };

    gtk_widget_show_all ( wd->window );

    gdk_monitor_get_workarea(
            gdk_display_get_monitor_at_window ( 
                gdk_window_get_display(gtk_widget_get_window(wd->window)),
                /* gdk_display_get_default(), */
                gtk_widget_get_window(wd->window)),
            &workarea);

    if ( cd->iconOffsetY < 0 && *targety < 0 ) {
        *targety = pointery - cd->iconOffsetY + 30;
        pcyan ( "重新调整y轴位置 y=%d", *targety);
    }
    else if ( cd->iconOffsetY > 0 && *targety+wd->height > workarea.height ) {
        *targety = pointery - wd->height - cd->iconOffsetY - 50;
        pcyan ( "重新调整y轴位置 y=%d", *targety);
    }

    return 0;
}

int moveWindow ( WinData *wd ) {

    ConfigData *cd = wd->cd;

    gint pointerX=100, pointerY=100;
    gint targetX = 100, targetY = 100;
    gint winWidth = 0, winHeight = 0;
    getPointerPosition ( &pointerX, &pointerY );
    /* gtk_window_get_size ( GTK_WINDOW(wd->window), */ 
    /*         &winWidth, &winHeight); */

    winWidth = wd->width;
    winHeight = wd->height;

    targetX  =  pointerX -
        ( cd->pointerOffsetX *1.0 / 400 ) * winWidth;

    targetY  = pointerY - 
        ( cd->pointerOffsetY *1.0 / 252 ) * winHeight;

    if ( cd->pointerOffsetY < 0 )
        targetY = pointerY - cd->pointerOffsetY;

    if ( cd->pointerOffsetY > 0 && targetY > pointerY + wd->height )
        targetY = pointerY + wd->height + cd->pointerOffsetY;

    pmag ( "Target window position %d %d, Original offset %d %d ",
            targetX, targetY, cd->pointerOffsetX, cd->pointerOffsetY );
    pmag ( "Win size: %d %d", winWidth, winHeight );

    if ( ! wd->quickSearchFlag ) {

        if ( cd->allowAutoAdjust ) 
            adjustTargetPosition( &targetX, &targetY, pointerX, pointerY, wd );

        pbcyan ( "Move window" );
        gtk_widget_hide ( wd->window );
        gdk_window_move ( 
                gtk_widget_get_window(wd->window),
                targetX, targetY );

        moveDone = 1;
    }


    return FALSE;
}

/* 本函数代码借鉴自xdotool部分源码*/
int focusOurWindow( WinData *wd ) {

    /* ConfigData *cd = wd->cd; */

    /* Get window id of x11*/
    GdkWindow *gw = gtk_widget_get_window ( GTK_WIDGET ( wd->window ) );
    if ( gw == NULL ) return 0;
    Window wid = gdk_x11_window_get_xid ( gw );
    if ( wid == 0 ) return 0;

    /* Get window's attributes*/
    XWindowAttributes wattr;
    Display *dpy = XOpenDisplay (NULL);
    if ( !dpy ) 
        return -1;

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

    if ( ret == 0 )
        pred("窗口聚焦请求失败(focusOurWindow)");

    XCloseDisplay(dpy);
    return 0;
}

int getFocusWinPos( int *x, int *y ) {

    gint wx = 0;
    gint wy = 0;
    gint win_x = 0;
    gint win_y = 0;

    GdkWindow *win=gdk_screen_get_active_window(gdk_screen_get_default());
    gdk_window_get_position (win,&wx,&wy);
    gdk_window_get_root_origin(win,&win_x,&win_y);

    *x = win_x;
    *y = win_y;

    /* pyellow ( "Current focus window pos: %d %d", *x, *y ); */

    /* win_width=gdk_window_get_width (win); */
    /* win_height=gdk_window_get_height(win); */

    return 0;
}

/* 此函数由于on_button_press_cb中拖动窗口的代码
 * 导致失效*/
gboolean on_button_release_cb ( 
        GtkWidget *widget,
        GdkEventButton *event,
        WinData *wd) {

    pyellow("Mouse Release");
    wd->mouseRelease = TRUE;
    return TRUE;
}

gboolean on_button_press_cb ( 
        GtkWidget *widget,
        GdkEventButton *event,
        WinData *wd) {

    if (event->type == GDK_BUTTON_PRESS)
    {
        if (event->button == 1) {
            gtk_window_begin_move_drag(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                    event->button,
                    event->x_root,
                    event->y_root,
                    event->time);
        }
    }

    wd->mousePress = TRUE;
    return TRUE;
}

int detect_outside_click_action ( void *data ) {

    WinData *wd = data;

    if ( moveDone || wd->quickSearchFlag ) {
        gtk_widget_show_all ( wd->window );
        focusOurWindow(wd);
    }

    ConfigData *cd = WINDATA(data)->cd;
    static gboolean block = FALSE;

    if ( ! action ){ return TRUE; } 

    if ( ! cd->alwaysDisplay ) return FALSE;

    /* block由函数末尾处识别到拖动窗口时使能,
     * 如果当前在拖动窗口并且鼠标没有释放，则直接
     * 返回TRUE，继续等待执行此函数内部逻辑
     *
     * 综合作用: 防止拖动窗口过快导致鼠标触及到窗口
     * 之外导致窗口意外关闭,
     *
     * 此处解决办法不是监听鼠标release事件是因为drag窗口动作导致
     * 此事件的回调函数没办法被执行到,应该是信号被阻断了*/
    if ( block && mouseNotRelease ) { return TRUE; }

    block = FALSE;

    /* GtkWidget *widget; */
    gint wx = 0;
    gint wy = 0;
    /* gint focusWinX = 0; */
    /* gint focusWinY = 0; */
    gboolean condition = FALSE;
    /* gboolean condition2 = FALSE;; */


    gtk_window_get_position ( 
            GTK_WINDOW(WINDATA(data)->window), &wx, &wy);

#if 1

    GdkWindow *win = gtk_widget_get_window(WINDATA(data)->window);
    gint w = gdk_window_get_width(win);
    gint h = gdk_window_get_height(win);
    gint pointerX = 0;
    gint pointerY = 0;
    gtk_window_get_size ( GTK_WINDOW(WINDATA(data)->window), &w, &h );

    getPointerPosition ( &pointerX, &pointerY );

    condition = 
        pointerX >= wx && pointerX <= wx+w &&
        pointerY >= wy && pointerY <= wy+h;

    /* condition满足，说明鼠标点击了窗口，并且
     * 鼠标未释放时，使能block变量*/
    if ( condition && mouseNotRelease )
        block = TRUE;

#endif

#if 0
    /* 这段代码会导致拖动翻译窗口的时候误关*/

    getFocusWinPos( &focusWinX, &focusWinY );

    /* 如果当前聚焦窗口坐标跟翻译窗口不同，则关闭翻译窗口
     * 有很小几率聚焦的窗口不是翻译窗口但是坐标又相同
     * 这里不予考虑*/
    condition2 = 
        wx == focusWinX && wy == focusWinY;

#endif

    /* if ( ! condition && ! condition2 ) */
    if ( ! condition ) {
        pbmag ( "区域外点击销毁窗口" );
        destroyNormalWin ( NULL, WINDATA(data) );
        return FALSE;
    }

    return TRUE;
}

int focus_request(void *data) {

    WinData *wd = data;

    /* 每隔一定时间多次尝试重新聚焦窗口防止聚焦窗口被抢占*/
    if ( focustimes <= 5 ) {
        focusOurWindow ( WINDATA(data) );
        focustimes++;
    }

    if ( focustimes > 5 ) {

        g_source_remove ( timeout_id );

        timeout_id = g_timeout_add (
                100,
                detect_outside_click_action,
                wd);
    }

    return TRUE;
}

int dataInit(WinData *wd) {

    /*Important: Pay attention to clear the values the global variables*/
    bw.width = 400; bw.height = 100; bw.lines = 0; bw.maxlen = 0;
    mw.width = 400; mw.height = 100; mw.lines = 0; mw.maxlen = 0;
    gw.width = 400; gw.height = 100; gw.lines = 0; gw.maxlen = 0;

    wd->bw = &bw; wd->gw = &gw; wd->mw = &mw;

    wd->image =  NULL;
    wd->oldImage = NULL;
    wd->srcBackgroundImage = NULL;
    wd->gdkwin = NULL;
    wd->width = 400;
    wd->height = 100;
    wd->hadRedirect = 0;
    wd->forceResize = 0;
    wd->calibrationButton = NULL;

    wd->drag = 0;
    wd->press = 0;
    wd->enter = 0;
    wd->ox = wd->oy = 0;
    wd->cx = wd->cy = 0;

    wd->quickSearchFlag = FALSE;
    wd->mousePress = FALSE;
    wd->mouseRelease = FALSE;

    moveDone = 0;

    return 0;
}

void makeSegmentationFault (  ) {
    char buf[1];
    if ( sizeof(buf) )  /* prevent anoying warning (set but not used)*/
        buf[9] = '0';
}


/*新建翻译结果窗口, 本文件入口函数*/
void *newNormalWindow ( void *data ) {

    pbred ( "启动Normal Win" );

    /* makeSegmentationFault(); */

    /* Storage the relative element or data in this window*/
    WinData wd;
    ConfigData *cd = data;

    wd.cd = cd;

    dataInit(&wd);

    focustimes = 1;
    InNewWin = 1;

    /* 窗口打开标志位 changed in captureShortcutEvent.c <变量shmaddr>*/
    shmaddr_keyboard[WINDOW_OPENED_FLAG] = '1';

    wd.gotOfflineTran = 0;
    wd.specific = 0;

    int ret = waitForContinue( &wd );

    if (ret ) {
        InNewWin = 0;
        return (void*)0;
    }

    /*新建并设置窗口基本属性*/
    gtk_init(NULL, NULL);

    GtkWidget *newWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_keep_above(GTK_WINDOW(newWin), TRUE);
    gtk_window_set_title(GTK_WINDOW(newWin), "");
    gtk_window_set_accept_focus ( GTK_WINDOW(newWin), TRUE );
    /* gtk_window_set_focus_on_map ( GTK_WINDOW(newWin), TRUE ); */
    gtk_widget_set_can_focus(newWin, TRUE);

    /* quickSearch快捷键标志位, changed in captureShortcutEvent.c <变量shmaddr>*/
    if ( shmaddr_keyboard[QUICK_SEARCH_NOTIFY] == '1') {
        shmaddr_keyboard[QUICK_SEARCH_NOTIFY] = '0';

        /* 快捷键调出的窗口放置于中央*/
        gtk_window_set_position(GTK_WINDOW(newWin), GTK_WIN_POS_CENTER);
        wd.quickSearchFlag = TRUE;
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


    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_view_set_buffer((GtkTextView*)view, buf);


    /*设置离左边以及顶部的距离*/
    gtk_text_view_set_left_margin ( (GtkTextView*)view, 10 );
    gtk_text_view_set_top_margin ( (GtkTextView*)view, 10 );

    /* window->layout->scroll->view*/
    gtk_container_add (GTK_CONTAINER(scroll), view);
    gtk_container_add (GTK_CONTAINER(layout), scroll);
    gtk_container_add (GTK_CONTAINER(newWin), layout);

    //gtk_container_add (GTK_CONTAINER(layout), view);
    //gtk_container_add (GTK_CONTAINER(scroll), layout);
    //gtk_container_add (GTK_CONTAINER(newWin), scroll);

    setFontProperties(buf, &iter);


    wd.view = view;
    wd.window = newWin;
    wd.layout = layout;

    /* Signals are used by everyone, but they are only created on a per class basis \
     * -- so you should not call call gtk_signal_new() unless you are writing a new \
     *  GtkObject type. However, if you want to make a new signal for an existing \
     *  type, you may use gtk_object_class_user_signal_new() to create a signal that\
     *  doesn't correspond to a class's builtin methods.
     * */

    /* 监听键盘事件的回调函数*/
    g_signal_connect ( newWin, "key-press-event", G_CALLBACK(on_key_press_cb), (void*)&wd );

    /* 这个获取背景图片并适应窗口的代码段放在分离翻译数据之前耗的时间才有
     * 意义，因为图片加载虽然花了一定时间，但总的还是会比翻译结果获取成功
     * 要少很多, 放在前面可以等到数据全部成功获取(一般来说百度的翻译结果会
     * 获取的比较慢)*/
    wd.image = syncImageSize ( newWin, wd.width, wd.height, (void*)&wd );

    if ( cd->hideHeaderBar )
    gtk_window_set_decorated ( GTK_WINDOW(newWin), FALSE );

    wd.exitButton = gtk_button_new_with_label ( "Exit" );
    gtk_layout_put ( GTK_LAYOUT(layout),
            wd.exitButton, bw.width-RIGHT_BORDER_OFFSET*6, bw.height-BOTTOM_OFFSET );
    g_signal_connect ( wd.exitButton, "clicked", G_CALLBACK(destroyNormalWin), &wd );


    pbred ( "打印调试信息" );
    printDebugInfo();


    pbred ( "初始化存储空间" );
    /*初始化百度以及离线翻译结果存储空间*/
    initMemoryBaidu();
    initMemoryMysql();
    initMemoryTmp();

    gtk_window_set_default_size (GTK_WINDOW(newWin), bw.width, bw.height);
    gtk_widget_set_size_request ( layout, bw.width, bw.height );
    gtk_widget_set_size_request (scroll, bw.width, bw.height);

    if ( google_result[0] == NULL )
        initMemoryGoogle();

    /* 百度翻译结果显示按钮*/
    GtkWidget *baiduButton = newBaiduButton( &wd );
    gtk_layout_put ( GTK_LAYOUT(layout), baiduButton, bw.width-RIGHT_BORDER_OFFSET, bw.height-BOTTOM_OFFSET );
    g_signal_connect(baiduButton, "clicked", G_CALLBACK(displayBaiduTrans), &wd);

    /* 谷歌翻译结果显示按钮*/
    GtkWidget *googleButton = newGoogleButton ( &wd );
    gtk_layout_put ( GTK_LAYOUT(layout), googleButton, bw.width-RIGHT_BORDER_OFFSET*2, bw.height-BOTTOM_OFFSET );
    g_signal_connect ( googleButton, "clicked", G_CALLBACK(displayGoogleTrans), &wd );

    /* 离线翻译结果切换按钮*/
    GtkWidget *mysqlButton = newOfflineButton ( &wd );
    gtk_layout_put ( GTK_LAYOUT(layout), mysqlButton, bw.width-RIGHT_BORDER_OFFSET*3, bw.height-BOTTOM_OFFSET );
    g_signal_connect ( mysqlButton, "clicked", G_CALLBACK(displayOfflineTrans), &wd );

    /* 校准按钮*/
    insertCalibrationButton(&wd);

    /* 指示当前翻译所属*/
    wd.indicateButton = newIndicateButton ( &wd );
    gtk_layout_put ( GTK_LAYOUT(layout), wd.indicateButton, bw.width-RIGHT_BORDER_OFFSET, bw.height-INDICATE_OFFSET );

    wd.buf = buf;
    wd.iter = &iter;
    wd.audio = NULL;
    wd.scroll = scroll;

    /*捕获resize window信号, 进行显示调整*/
    g_signal_connect (newWin, "configure-event", 
            G_CALLBACK(syncNormalWinForConfigEvent), &wd);
    g_signal_connect (newWin, "button-press-event", 
            G_CALLBACK(on_button_press_cb), &wd);
    g_signal_connect (newWin, "button-release-event", 
            G_CALLBACK(on_button_release_cb), &wd);


    pbred ( "聚焦请求" );
    timeout_id = g_timeout_add(10, focus_request, &wd);

    if ( ! wd.quickSearchFlag )
        movewindow_timeout_id = g_timeout_add ( 100, (int(*)(void*))moveWindow, &wd);

    pbred ( "显示所有" );
    /* gtk_widget_show_all(newWin); */

    /* GtkTextView 插入文本时的回调函数注册*/
    //g_signal_connect ( buf, "changed", G_CALLBACK(text_changed), (void*)&wd );

    pbred ( "选择显示界面" );
    selectDisplay ( &wd );

    pbred ( "主循环" );
    gtk_main();
    pbred ( "after gtk_main" );

    pbcyan ( ">>>InNewWin 置零" );
    InNewWin = 0;
    shmaddr_keyboard[WINDOW_OPENED_FLAG] = '0';

    pbred ( "关闭 Normal win" );
    pthread_exit(NULL);
}

int destroyNormalWin(GtkWidget *unKnowWidget, WinData *wd) {

    pbblue ( "Destroy window" );

    clearMemory();

    g_source_remove ( timeout_id );

    /* 窗口关闭标志位*/
    shmaddr_keyboard[WINDOW_OPENED_FLAG] = '0';
    shmaddr_keyboard[CTRL_C_PRESSED_FLAG] = '0';

    //gtk_widget_destroy ( wd->image );

    /*清除相关标记*/
    shmaddr_baidu[0] = CLEAR;
    shmaddr_google[0] = CLEAR;

    /* TODO:按了exit键后变成了单击事件，此时再双击会导致检测错误
     * 应手动置0 ( 当前可以不用这个了，这是以前用过的,不过先放着
     * 可能以后用得着，可以当个提醒 )*/
    action = 0;

    pbcyan ( "InNewWin 置零" );

    /* 已退出翻译结果窗口，重置标志变量*/
    InNewWin = 0;


    //gtk_window_close(GTK_WINDOW(window));
    gtk_widget_destroy(wd->window);
    gtk_main_quit();

    return FALSE;
}

/*Get index of separate symbols*/
int getIndex(int *index, char *src) {

    if ( ! tmp ){
        pbred ( "临时内存未初始化" );
        return -1;
    }

    pbyellow ( "getindex:%s", src );
    clearBaiduMysqlResultMemory();

    strcpy ( tmp, src );
    char *p = &tmp[ACTUALSTART];
    int i = ACTUALSTART;  /*同p一致指向同一个下标字符*/
    int charNum = 0;

    if ( !*p )
        return -1;

    while ( *p ) 
    {
        if ( *p == '|' ) 
        {
            *p = '\0';

            /*截取到第三个分隔符*/
            if ( src == shmaddr_google && charNum >= 2 )
                break;

            index[charNum++] = i + 1; /*记录字符串下标*/
        }
        p++; i++;
    }

    return 0;
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
    wd->gotOfflineTran = ( shmaddr_mysql[0] == FINFLAG ) ? 1 : 0;
    wd->gotGoogleTran = ( shmaddr_google[0] == FINFLAG ) ? 1 : 0;
    wd->gotBaiduTran = ( shmaddr_baidu[0] == FINFLAG ) ? 1 : 0;

    /*原始数据超过一定长度，在ScrolledWin中显示, 并返回1，
     * 不再执行newWin函数*/
    if ( strlen (text) > 130 ) 
        return newScrolledWin();

    return 0;
}
/* 重新从共享内存获取百度翻译结果并设置窗口大小*/
int reGetBaiduTrans (gpointer *data, int who ) {

    int index[INDEX_SIZE] = { 0 };
    int ret = 0;
    ret = getIndex(index, GET_SHMADDR(who) );
    if ( ret ) {
        return ret;
    }

    int len = GET_DISPLAY_LINES_NUM ( data, who ) > 15 ? 28 : 28;
    pbred ( "单行长度=%d", len );
    separateDataForBaidu(index, len, TYPE(who) );

    return 0;
}

int adjustWinSize(GtkWidget *button, gpointer *data, int who ) {

    int ret = 0;

    if ( who == GOOGLE ) 
    {
        int index[2] = { 0 };
        ret = getIndex(index, shmaddr_google);
        if ( ret ) {
            return ret;
        }

        /* 找到分割符，数据分离提取才有意义*/
        if ( index[0] != 0 )
            separateGoogleData ( index );
        else
            pred("未找到分隔符(adjustWinSize)");
    }
    else if ( who == BAIDU || who == MYSQL)
    {
        /*还未获取到结果，应重新获取并设置窗口大小*/
        //if ( strlen ( ZhTrans(TYPE(who), 0) ) == 0) {
        ret = reGetBaiduTrans ( data, who );
        if ( ret ) return ret;
        //}
    }

    ret = setWinSizeForNormalWin (WINDATA(data), GET_SHMADDR(who), TYPE(who));
    if ( ret ) return ret;

    return 0;
}

static inline int nextWindow ( int who )  {
    return who < 3 ? who + 1 : 1;
}

/* 切换各个翻译结果的显示*/
int changeDisplay(GtkWidget *button, gpointer *data) {

    WINDATA(data)->who = nextWindow((WINDATA(data)->who));

    if ( WINDATA(data)->who == BAIDU )
        displayBaiduTrans( button, data );
    else if ( WINDATA(data)->who == GOOGLE )
        displayGoogleTrans(button, data);
    else if ( WINDATA(data)->who == OFFLINE )
        displayOfflineTrans(button, data);

    return 0;
}

void displayGoogleTrans(GtkWidget *button, gpointer *data) {

    pyellow("\n显示谷歌翻译结果:\n\n");

    WINDATA(data)->who = GOOGLE;
    WINDATA(data)->specific = 1;

    /* 调整窗口大小*/
    adjustWinSize ( button, data, GOOGLE );

    gint width = WINDATA(data)->width;
    gint height = WINDATA(data)->height;

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,(WINDATA(data))->indicateButton,\
            width-(200-(RIGHT_BORDER_OFFSET*(WINDATA(data))->who)), height-INDICATE_OFFSET );
    gtk_widget_queue_draw( WINDATA(data)->window );

    if ( WINDATA(data)->audio != NULL )
        gtk_widget_hide ( WINDATA(data)->audio  );

    GtkTextIter *iter, start, end;
    iter = WINDATA(data)->iter;
    GtkTextBuffer *buf = WINDATA(data)->buf;

    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_get_end_iter(buf, &end);

    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);

    WINDATA(data)->iter = iter ;

    syncImageSize ( WINDATA(data)->window,\
            WINDATA(data)->width,WINDATA(data)->height,  data) ;

    char enter[] = "\n";

    /*插入输入原文*/
    if ( strlen( text )  < 30 ) {

        gtk_text_buffer_insert_with_tags_by_name(buf, iter, text, -1, 
                "black-font",  "bold-style",  "font-size-15", "underline", NULL);

        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
        //gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
    }

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

    WINDATA(data)->who = MYSQL;
    WINDATA(data)->specific = 1;

    adjustWinSize ( button, data, MYSQL );

    gint width = WINDATA(data)->width;
    gint height = WINDATA(data)->height;

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,(WINDATA(data))->indicateButton,\
            width-(200-(RIGHT_BORDER_OFFSET*(WINDATA(data))->who)), height-INDICATE_OFFSET );
    gtk_widget_queue_draw( WINDATA(data)->window );

    GtkTextIter *iter, start, end;
    iter = WINDATA(data)->iter;
    GtkTextBuffer *buf = WINDATA(data)->buf;

    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_get_end_iter(buf, &end);

    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);

    WINDATA(data)->iter = iter ;

    if ( ! WINDATA(data)->gotOfflineTran && strlen ( ZhTrans(OFFLINE, 0) ) == 0 )
        gtk_text_buffer_insert_with_tags_by_name(buf, iter, "\n  NOT FOUND ANYTHING",\
                -1, "yellow-font",  "heavy-font", \
                "font-size-11", "letter-spacing", NULL);

    syncAudioBtn ( WINDATA(data), OFFLINE );

    char enter[] = "\n";

    /*根据得到的相关结果进行翻译内容输出*/
    for ( int i=0; i<BAIDUSIZE-1; i++ ) {

        /* 翻译结果不为空*/
        if ( mysql_result[i][0][0] != '\0') {

            /* 翻译结果输出控制代码段*/
            if ( i == 0 && strlen(mysql_result[i][0]) < 30 ) {

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, mysql_result[i][0], \
                        -1,"black-font",  "bold-style", \
                        "font-size-15", "letter-spacing","underline", NULL);
            }
            else if ( i == 1 ) {

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, mysql_result[i][0],\
                        -1, "blue-font",  "heavy-font", \
                        "font-size-11", "letter-spacing", NULL);

            }
            else if ( i == 4 ) {

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, mysql_result[i][0],\
                        -1, "brown-font",  "heavy-font", \
                        "font-size-11","letter-spacing", NULL);
            }
            else if ( i != 0 ) {

                if ( strlen(Phonetic(OFFLINE)) == 0 && strlen(ZhTrans(OFFLINE, 0)) != 0\
                        && strlen(EnTrans(OFFLINE)) == 0 && strlen(OtherWordForm(OFFLINE)) == 0){

                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, mysql_result[i][0],\
                            -1, "brown-font",  "heavy-font", \
                            "font-size-11", "letter-spacing", NULL);

                } else {

                    for ( int j=0; j<ZH_EN_TRAN_SIZE && mysql_result[i][j][0]; j++ )
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, mysql_result[i][j],\
                                -1, "green-font",  "heavy-font", \
                                "font-size-11", "letter-spacing", NULL);
                }

            }

            /* 回车符控制输出代码段*/
            if ( i == 0 )  {

                /* 只有源输入而之后没结果了，插入一个回车符*/
                if (( strlen(mysql_result[3][0]) ==0 && strlen(mysql_result[4][0]) == 0\
                            && strlen(mysql_result[2][0]) == 0 && strlen(mysql_result[1][0]) == 0)) 
                {
                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                }

                else  
                {   
                    if (!(strlen(Phonetic(OFFLINE)) == 0 && strlen(ZhTrans(OFFLINE,0)) != 0 && strlen(EnTrans(OFFLINE)) == 0\
                                && strlen(OtherWordForm(OFFLINE)) == 0 && strlen(ZhTrans(OFFLINE,0)) != 0) ){
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                    }
                    else
                    {
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                    }
                }
            }

            else if ( i == 1 && strlen(mysql_result[1][0]) != 0 && ( strlen(mysql_result[2][0]) != 0\
                        || strlen(mysql_result[3][0]) != 0 || strlen(mysql_result[4][0]) != 0))

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);

            else if ( i == 2 && ( strlen(mysql_result[3][0]) != 0 || strlen(mysql_result[4][0]) != 0 ) )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);


            else if ( i == 3 && ( strlen(mysql_result[4][0]) != 0) )
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

    GtkTextBuffer *buf = WINDATA(data)->buf;

    if ( strlen (ZhTrans(ONLINE,0)) == 0 && strlen ( EnTrans(ONLINE) ) == 0 ) {

        pred("ZhTrans(ONLINE,0) & EnTrans(ONLINE)长度皆为0 \n");
        reGetBaiduTrans ( data, BAIDU );
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
    syncAudioBtn ( WINDATA(data), ONLINE );

    char enter[] = "\n";

    /*根据得到的相关结果进行翻译内容输出*/
    for ( int i=0; i<BAIDUSIZE-1; i++ ) {

        /* 翻译结果不为空*/
        if ( baidu_result[i][0][0] != '\0') {

            /* 翻译结果输出控制代码段*/
            if ( i == 0 && strlen(baidu_result[i][0]) < 30 ) {

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i][0], \
                        -1,"black-font",  "bold-style", \
                        "font-size-15", "letter-spacing","underline", NULL);
            }
            else if ( i == 1 ) {

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i][0],\
                        -1, "blue-font",  "heavy-font", \
                        "font-size-11", "letter-spacing", NULL);
            }
            else if ( i == 4 ) {

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i][0],\
                        -1, "brown-font",  "heavy-font", \
                        "font-size-11","letter-spacing", NULL);
            }
            else if ( i != 0 ) { /* i==2 || i == 3*/

                /* 翻译结果只有一句(如短句翻译会出现这种情况),显示设置为棕色*/
                if ( strlen(Phonetic(ONLINE)) == 0 && strlen(ZhTrans(ONLINE,0)) != 0\
                        && strlen(EnTrans(ONLINE)) == 0 && strlen(OtherWordForm(ONLINE)) == 0){

                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i][0],\
                            -1, "brown-font",  "heavy-font", \
                            "font-size-11", "letter-spacing", NULL);

                } else {

                    for ( int j=0; j<ZH_EN_TRAN_SIZE && baidu_result[i][j][0]; j++ )
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i][j],\
                                -1, "green-font",  "heavy-font", \
                                "font-size-11", "letter-spacing", NULL);
                }

            }

            /* 回车符控制输出代码段*/
            if ( i == 0 )  {

                /* 只有源输入而之后没结果了，插入一个回车符*/
                if (( strlen(baidu_result[3][0]) ==0 && strlen(baidu_result[4][0]) == 0\
                            && strlen(baidu_result[2][0]) == 0 && strlen(baidu_result[1][0]) == 0)) 
                {
                    gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                }

                else  
                {   
                    if (!(strlen(Phonetic(ONLINE)) == 0 && strlen(ZhTrans(ONLINE,0)) != 0 && strlen(EnTrans(ONLINE)) == 0\
                                && strlen(OtherWordForm(ONLINE)) == 0 && strlen(ZhTrans(ONLINE,0)) != 0) ){
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                    }
                    else
                    {
                        gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                    }
                }
            }

            else if ( i == 1 && strlen(baidu_result[1][0]) != 0 && ( strlen(baidu_result[2][0]) != 0\
                        || strlen(baidu_result[3][0]) != 0 || strlen(baidu_result[4][0]) != 0))

                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);

            else if ( i == 2 && ( strlen(baidu_result[3][0]) != 0 || strlen(baidu_result[4][0]) != 0 ) )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);


            else if ( i == 3 && ( strlen(baidu_result[4][0]) != 0) )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
        } 

        /* 翻译结果检测为空*/
        //else if ( i == 0  && Phonetic(ONLINE)Flag == 0 && NumZhTranFlag==0
        /*&& NumEnTranFlag == 0 && OtherWordForm(ONLINE)Flag == 0) */

        /* 这里不要再用上面的用标志位检测，存在一种情况，翻译结果没复制过来，上面的结果插入语句无法执行
         * 刚好到这里的时候标志位又被刚好翻译结果写入完成的python端修改，导致重定向失败，这里的所有逻辑
         * 都不要用标志位判断*/
        else if ( i == 0  && strlen(Phonetic(ONLINE)) == 0 && strlen(ZhTrans(ONLINE,0))==0 \
                && strlen(EnTrans(ONLINE)) == 0 && strlen(OtherWordForm(ONLINE)) == 0){

            gtk_text_buffer_insert_with_tags_by_name(buf, iter, "\n\n   必应翻译尚未获取成功,或输入内容不支持\n   (只允许翻译单词)",\
                    -1, "brown-font",  "heavy-font", \
                    "font-size-11","letter-spacing", NULL);
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
}

/*当窗口大小被鼠标改变时进行窗口重绘以及自动调整switch button位置*/
void syncNormalWinForConfigEvent( GtkWidget *window, GdkEvent *event, gpointer data ) {

    gint width, height;
    static unsigned int lastwidth = 0, lastheight = 0;

    gtk_window_get_size ( (GtkWindow*)window, &width, &height );

    if (WINDATA(data)->width <= width &&  WINDATA(data)->height <= height)
        WINDATA(data)->specific = 0;

    /* 窗口大小未改变不用重新调整布局,直接返回*/
    if ( lastwidth == width && lastheight == height && !WINDATA(data)->specific){
        return;
    }

    /* 指定窗口大小时*/
    if ( WINDATA(data)->specific ) {
        width = WINDATA(data)->width;
        height = WINDATA(data)->height;
#if 0
        gtk_widget_set_size_request ( (GtkWidget*)(WINDATA(data))->layout,  width, height);
#endif

    }

    lastwidth = width;
    lastheight = height;
    syncImageSize ( (WINDATA(data))->window,width,height, data );

    gtk_window_resize ( GTK_WINDOW((WINDATA(data))->window), width, height );
    gtk_widget_set_size_request ( (GtkWidget*)(WINDATA(data))->scroll,  width, height);

    /* layout不要在这重设大小，否则将造成无法在缩小窗口*/
    //gtk_widget_set_size_request ( (GtkWidget*)(WINDATA(data))->layout,  width, height);

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout, \
            (WINDATA(data))->baiduButton, width-RIGHT_BORDER_OFFSET, height-BOTTOM_OFFSET );

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,\
            (WINDATA(data))->googleButton, width-RIGHT_BORDER_OFFSET*2, height-BOTTOM_OFFSET );

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,\
            (WINDATA(data))->mysqlButton, width-RIGHT_BORDER_OFFSET*3, height-BOTTOM_OFFSET );

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,\
            (WINDATA(data))->calibrationButton, width-RIGHT_BORDER_OFFSET*4, height-BOTTOM_OFFSET );

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,\
            (WINDATA(data))->exitButton, width-RIGHT_BORDER_OFFSET*5-20, height-BOTTOM_OFFSET );

    gtk_layout_move ( (GtkLayout*)(WINDATA(data))->layout,(WINDATA(data))->indicateButton,\
            width-(200-(RIGHT_BORDER_OFFSET*(WINDATA(data))->who)), height-INDICATE_OFFSET );

    WINDATA(data)->width = width;
    WINDATA(data)->height = height;

    WINDATA(data)->lastwidth = width;
    WINDATA(data)->lastheight = height;

    gtk_widget_queue_draw ( window );
}

int calculateWidth ( int x ) {

    double wa, wb, wc, wd; /* For width*/
    genFitFunc ( "winSizeInfo_part1", FITTING_STATUS );
    getFitFunc ( expanduser("/home/$USER/.stran/winSizeInfo_part1.func"),\
            FOR_WIN_WIDTH, &wa, &wb, &wc, &wd, FITTING_STATUS );

    return  (int) ( wa*x*x*x + wb*x*x +wc*x +wd + 0.5 );
}

int calculateHeight ( int x ) {

    double ha, hb, hc, hd; /* For height*/
    genFitFunc ( "winSizeInfo_part2", FITTING_STATUS );
    getFitFunc ( expanduser("/home/$USER/.stran/winSizeInfo_part2.func"),\
            FOR_WIN_HEIGHT, &ha, &hb, &hc, &hd, FITTING_STATUS );

    return  (int) ( ha*x*x*x + hb*x*x +hc*x +hd + 0.5 );
}

int limitSize ( double *width, double *height ) {

    /*别让窗口过小,或过大*/
    //if ( *width < 400 ) *width = 400;
    if ( *height < 100 ) *height = 100;
    if ( *width > 1000 ) *width = 1000;
    if ( *height > 900 ) *height = 900;

    return 0;
}

/* 设置NormalWin的窗口大小, type: ONLINE / OFFLINE*/
int setWinSizeForNormalWin (WinData *window, char *addr, int type) {

    int maxlen = 0;
    int lines = 0;

    if ( addr == shmaddr_baidu || addr == shmaddr_mysql) {

        maxlen = getMaxLenOfBaiduTrans ( type );
        lines = getLinesNumOfBaiduTrans ( type );
    }
    else if ( addr == shmaddr_google ) {

        lines = getLinesNumOfGoogleTrans();
        maxlen = getMaxLenOfGoogleTrans();
    }

    if ( maxlen <= 0 || lines <= 0 )
        return -1;

    STORE_DISPLAY_MAX_LEN ( window, WHO(addr), maxlen );
    STORE_DISPLAY_LINES_NUM ( window, WHO(addr), lines );

    double width, height;
    width = calculateWidth ( maxlen );
    height = calculateHeight ( lines );

    pbblue("maxlen=%d lines=%d", maxlen, lines);
    pbblue("width=%d height=%d", (int)width, (int)height);

    limitSize ( &width, &height );

    /*Update the window size only when the new size is larger than older's*/
    STORE_DISPLAY_WIDTH(WHO(addr), width);
    STORE_DISPLAY_HEIGHT(WHO(addr), height);

    if ( WINDATA(window)->width < GET_DISPLAY_WIDTH(WHO(addr)) )
        WINDATA(window)->width = GET_DISPLAY_WIDTH(WHO(addr));

    if (WINDATA(window)->height < GET_DISPLAY_HEIGHT(WHO(addr)))
        WINDATA(window)->height = GET_DISPLAY_HEIGHT(WHO(addr));

    if ( WINDATA(window)->lastheight <= WINDATA(window)->height 
            || WINDATA(window)->width <= WINDATA(window)->lastwidth )
        WINDATA(window)->specific = 1;

    if ( WINDATA(window)->specific == 1 )
        syncNormalWinForConfigEvent ( WINDATA(window)->window, NULL, window );

    return 0;
}
