#include "common.h"

extern char *shmaddr;
extern int action;

int destroy_newwin(GtkWidget *window);
void getShmDate(int (*index)[]);

/*新建翻译结果窗口*/
void *newWindow(void * arg) {

    int flag = 0;

    /* 置零action，用于下面翻译窗口弹不出时可以双击关闭*/
    action = 0; 

    printf("new window func in newWindow.c\n");

    /*等待python端的翻译数据全部写入共享内存*/
    while( shmaddr[0] != FINFLAG) {

        if ( flag ) {
            flag = 0;
            printf("准备接收共享内存数据...\n");
        }

        /*长时间未检测到共享内存里的数据进行双击或者单机取消本次窗口显示*/
        if ( action == DOUBLECLICK/* || action == SINGLECLICK*/) {

            printf("捕获双击退出: In newWindow.c\n");

            /*此处action只代表取消显示，应重置action*/
            action = 0;
            return (void*)0;
        }
        if ( shmaddr[0] == ERRCHAR) {
            printf("翻译过程出现错误\n");
            printf("准备窗口错误提示\n");
            break;
        }

        if ( shmaddr[0] == NULLCHAR || shmaddr[0] == EXITFLAG) {
            if ( shmaddr[0] == NULLCHAR)
                printf("空字符串\n");
            else {
                printf("Note: 翻译程序程序已退出,请不要再往下执行\n");
                printf("退出程序...\n");
                exit(0);
            }
            action = 0;
            shmaddr[0] = CLEAR;
            return (void*)0;
        }
        usleep(400000);
    }

    /*新建并设置窗口基本属性*/
    gtk_init(NULL, NULL);
    GtkWidget *newWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_keep_above(GTK_WINDOW(newWin), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(newWin), 550, 334);
    gtk_window_set_title(GTK_WINDOW(newWin), "");
    gtk_window_set_position(GTK_WINDOW(newWin), GTK_WIN_POS_MOUSE);
    gtk_window_set_resizable(GTK_WINDOW(newWin), FALSE);

    /*新建box容纳文字显示*/
    GtkWidget *vbox;
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

    g_signal_connect(newWin, "destroy", G_CALLBACK(destroy_newwin), newWin);

    int index[2] = { 0 };

    if ( shmaddr[0] != ERRCHAR)
        /* 从共享内存中截取出部分翻译结果(3条结果),
         * 后两条存于索引数组index中*/
        getShmDate(&index);

    printf("目标翻译:%s\n", &shmaddr[1]);
    printf("释义:%s\n", &shmaddr[index[0]]);
    printf("相关:%s\n", &shmaddr[index[1]]);

    /*创建layout用于显示背景图片,以及放置文本*/
    GtkWidget * layout = gtk_layout_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(newWin), layout);
    gtk_widget_show(layout);

    /*建立文字显示区域*/
    GtkWidget *view;
    GtkTextBuffer *buf;
    GtkTextIter iter;

    /*GtkTextView需要放在box里，不然弹出窗口时不显示*/
    view = gtk_text_view_new();
    gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 25);

    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);

    GdkPixbuf *src = gdk_pixbuf_new_from_file(\
            "/home/rease/Pictures/silhouette_moon_boat_135277_3840x2160.jpg", NULL);

    GdkPixbuf *dst = gdk_pixbuf_scale_simple(src, 550, 334, GDK_INTERP_BILINEAR);
    GtkWidget *image = gtk_image_new_from_pixbuf(dst);
    gtk_layout_put(GTK_LAYOUT(layout), image, 0, 0);
    gtk_layout_put(GTK_LAYOUT(layout), vbox, 10, 10);

    g_object_unref(src);
    g_object_unref(dst);

    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_view_set_buffer((GtkTextView*)view, buf);

    /*注意属性值设置正确，不然桌面分分钟崩溃:(*/
    //gtk_text_buffer_create_tag(buf, "blue-font", "foreground", "#0055ff", NULL);
    gtk_text_buffer_create_tag(buf, "blue-font", "foreground", "#0000ff", NULL);
    gtk_text_buffer_create_tag(buf, "bold-style", "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag(buf, "Uneditable", "editable", FALSE, NULL);
    gtk_text_buffer_create_tag(buf, "font-size", "font", "13", NULL );
    gtk_text_buffer_create_tag(buf, "gray_background", "background", "#ffffff", NULL);
    gtk_text_buffer_get_iter_at_offset(buf, &iter, 0);


    /*调整字符串*/
    char *p[3] ={ NULL };
    p[0] = &shmaddr[1];
    p[1] = &shmaddr[index[0]];
    p[2] = &shmaddr[index[1]];

    char result[1024] = { '\0' };
    char explain[4096] = { '\0' };
    char related[2048] = { '\0' };

    char *storage[3] = { NULL };
    storage[0] = result;
    storage[1] = explain;
    storage[2] = related;

    if ( shmaddr[0]  != ERRCHAR )
        /*主要完成加入回车符使单行句子不至于太长*/
        adjustStr(p, 65, storage);
    else  {
        shmaddr[0] = CLEAR;
        strcpy(storage[0], "翻译超时或出现其他错误");
    }

    /*插入翻译结果*/
    char doubleEnter[] = "\n\n";
    char enter[] = "\n";
    for ( int i=0; i<3; i++ ) {
        printf("storage[]=%s\n", storage[i]);
        if ( storage[i][0] != '\0') {

            gtk_text_buffer_insert_with_tags_by_name(buf, &iter, storage[i], -1, 
                    "blue-font", "gray_background", "bold-style", "Uneditable", "font-size", NULL);

            if ( i<2 && storage[i+1][0] != '\0')
                gtk_text_buffer_insert_with_tags_by_name(buf, &iter, doubleEnter, -1, NULL, NULL);
            else 
                gtk_text_buffer_insert_with_tags_by_name(buf, &iter, enter, -1, NULL, NULL);
        }
    }

    gtk_widget_set_opacity(image, 0.7);
    gtk_widget_set_opacity(view, 0.6);
    //gtk_widget_set_opacity(vbox, 0.4);

    /*显示*/
    gtk_widget_show_all(newWin);
    gtk_main();

    /*退出窗口，设置新建窗口标志为0(不能新建)*/
    printf("new window quit...\n");

    pthread_exit(NULL);
}

int destroy_newwin(GtkWidget *window) {

    /*标记已退出newWindow函数*/
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

void getShmDate(int (*index)[]) {

    char *p = &shmaddr[1];
    int i = 1;  /*同p一致指向第一个下标字符*/
    int charNum = 0;

    printf("原始数据:%s\n", &shmaddr[1]);

    while ( *p && i < SHMSIZE) 
    {
        if ( *p == '|' ) 
        {
            *p = '\0';
            if ( charNum >= 3 ) /*截取到第三个分隔符*/
                break;

            (*index)[charNum++] = i + 1; /*记录字符串下标*/
        }
        p++;
        i++;
    }
    shmaddr[0] = '\0';
    printf("%d %d %d\n", (*index)[0], (*index)[1], (*index)[2]);
}
