#include "common.h"
#include "newWindow.h"

int scrollShow = -1;

/*跟上一个函数是一样的, 这里主要是考虑到后期拓展和区分窗口写成了两个函数*/
void syncScrolledWinWithConfigEvent ( GtkWidget *window, GdkEvent *event, gpointer *wd ) {

    gint width, height;
    static unsigned int lastwidth = 0, lastheight = 0;

    gtk_window_get_size ( (GtkWindow*)window, &width, &height );

    /* 窗口大小未改变不用重新调整布局,直接返回*/
    if ( lastwidth == width && lastheight == height )
        return;

    lastwidth = width;
    lastheight = height;

    gtk_window_resize ( GTK_WINDOW(((WinData*)wd)->window), width, height );
    gtk_widget_set_size_request ( (GtkWidget*)((WinData*)wd)->scroll,  width, height);
    gtk_layout_move ( (GtkLayout*)((WinData*)wd)->layout, ((WinData*)wd)->switchButton,\
            width-RIGHT_BORDER_OFFSET, height-BOTTOM_OFFSET );

    syncImageSize ( ((WinData*)wd)->window,WINDATA(wd)->width,WINDATA(wd)->height, (void*)wd );
    ((WinData*)wd)->width = width;
    ((WinData*)wd)->height = height;

    gtk_widget_queue_draw ( window );
    gtk_widget_show_all ( ((WinData*)wd)->window );
}

void resizeScrolledWin ( WinData *wd, gint width, gint height ) {

    //wd->forceResize = 1;
    wd->height = height;
    wd->width = width;
    syncImageSize ( wd->window, wd->width, wd->height, (void*)wd );

    gtk_window_resize ((GtkWindow*) wd->window, width, height );
    gtk_widget_set_size_request ( wd->scroll,  width, height);

    gtk_layout_move ( (GtkLayout*)wd->layout, wd->switchButton, width-RIGHT_BORDER_OFFSET, height-BOTTOM_OFFSET );
    gtk_widget_queue_draw ( wd->window );

    gtk_widget_show_all(wd->window);

    gtk_window_get_size ( (GtkWindow*)(wd->window), &width, &height );
    pyellow("now size %d %d \n", width, height);

}

/* Suit the size of window with the number of characters */
void suitWinSizeWithCharNum ( char *addr , WinData *wd) {

    if ( addr == shmaddr_baidu ) {

        int charNums = countCharNums ( &shmaddr_baidu[ACTUALSTART+2] );

        pgreen(" (showBaiduScrolledWin) charNums=%d\n", charNums);

        /* 根据字符数量控制窗口大小和单行长度*/
        if ( charNums < 400 ) {

            int lines = countLines ( 30 , &shmaddr_baidu[ACTUALSTART+2] );
            pgreen(" (showBaiduScrolledWin) lines=%d\n", lines);
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
            pgreen("baiduRestlt countLines=%d \n", lines);
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
        pyellow("charNums=%d \n", charNums);

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

    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter (gtbuf, &start);
    gtk_text_buffer_get_end_iter (gtbuf, &end);
    gtk_text_buffer_delete (gtbuf, &start, &end);
    gtk_text_buffer_get_iter_at_offset (gtbuf, iter, 0);

    int index[13] = { 0 };
    getIndex(index, shmaddr_baidu);

    /* getIndex 会将分隔符修改为'\0', 再一次进入这个函数index[1]会成0
     * 所以这里是为了区分是否是第一次进入这里，如果不是，插入已经保存
     * 到ZhTrans(ONLINE)的数据, 否则先生成ZhTrans(ONLINE)*/
    if ( index[1] != 0 ) {

        shmaddr_baidu[0] = '0';

        suitWinSizeWithCharNum ( shmaddr_baidu , wd );

        resizeScrolledWin ( wd, wd->width, wd->height );
        pgreen(" (showBaiduScrolledWin) width=%d height=%d\n", wd->width, wd->height);

        strcpy ( ZhTrans(ONLINE),  &shmaddr_baidu[2+ACTUALSTART]);

        pgreen(" 调整后输出的百度翻译结果:%s\n", ZhTrans(ONLINE));

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

        pgreen(" ZhTrans(ONLINE)里的结果输出\n");

        gtk_text_buffer_insert_with_tags_by_name ( gtbuf, iter, ZhTrans(ONLINE), -1,\
                "brown-font", "font-size-14", "bold-style", NULL );
    }
    else {

        pred(" 未获取到百度翻译结果\n");

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

    pyellow("谷歌翻译结果:%s\n\n", &shmaddr_google[ACTUALSTART]);

    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter (gtbuf, &start);
    gtk_text_buffer_get_end_iter (gtbuf, &end);
    gtk_text_buffer_delete (gtbuf, &start, &end);
    gtk_text_buffer_get_iter_at_offset (gtbuf, iter, 0);

    /* 仍旧包含分隔符，说明是新的字符串翻译结果，未显示，需要调整*/
    if ( isContainSeparateChar ( &shmaddr_google[ACTUALSTART] ) )
        wd->hadShowGoogleResult = 0;

    if ( ! wd->hadShowGoogleResult ) {

        suitWinSizeWithCharNum ( shmaddr_google, wd );

        pyellow("window width,height %d %d \n", wd->width, wd->height);

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

    GtkWidget *button = newSwitchButton( (WinData*)&wd );

    gtk_layout_put ( (GtkLayout*)layout, button, width-RIGHT_BORDER_OFFSET, height-BOTTOM_OFFSET );

    GtkTextIter iter;

    wd.layout = layout;
    wd.switchButton = button;
    wd.scroll = scroll;
    wd.buf = gtb;
    wd.width = width;
    wd.height = height;
    wd.iter = &iter;
    wd.window = window;
    wd.hadShowGoogleResult = 0;

    wd.oldImage = NULL;
    wd.srcBackgroundImage = NULL;
    wd.image = syncImageSize ( window, wd.width, wd.height, (void*)&wd );

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
    shmaddr_keyboard [WINDOW_OPENED_FLAG] = '0';

    return 1;
}
