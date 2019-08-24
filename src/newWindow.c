#include "common.h"

extern char *shmaddr_google;
extern char *shmaddr_baidu;
extern int action;

char *baidu_result[BAIDUSIZE] = { NULL };

int InNewWin = 0;
int show = -1;
extern int CanNewEntry;
extern int lines;
extern int maxlen_baidu;

#define PhoneticFlag ( shmaddr_baidu[1] - '0' )
#define NumZhTranFlag ( shmaddr_baidu[2] - '0')
#define NumEnTranFlag ( shmaddr_baidu[3] - '0')
#define OtherWordFormFlag ( shmaddr_baidu[4] - '0')
#define NumAudioFlag ( shmaddr_baidu[5] - '0')

#define SourceInput ((char *)( baidu_result[0] ))
#define Phonetic ((char *)( baidu_result[1] ))
#define ZhTrans ((char *)( baidu_result[2] ))
#define EnTrans ((char *)( baidu_result[3] ))
#define OtherWordForm ((char *)( baidu_result[4] ))
#define Audios ((char *)( baidu_result[5] ))

int destroy_newwin(GtkWidget *window);
int waitForContinue();
void getIndex(int *index, char *addr);
void get_paragraph();
void initMemory();
void printDebugInfo();

void setFontProperties(GtkTextBuffer *buf, GtkTextIter *iter);

void change_display(GtkWidget *button, gpointer *arg);
void displayGoogleTrans(GtkWidget *button, gpointer *arg);
void displayBaiduTrans(GtkTextBuffer *buf, GtkTextIter* iter);

struct GtkText {
    GtkTextIter *iter;
    GtkTextBuffer *buf;
    int index_google[2];
    char **storage;
};

/*新建翻译结果窗口*/
void *newWindow(void * arg) {

    InNewWin = 1;

    /* 置零action，用于下面翻译窗口弹不出时可以双击关闭*/
    action = 0; 

    printf("\n准备判断是否新建窗口\n\n");
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

    g_signal_connect(newWin, "destroy", G_CALLBACK(destroy_newwin), newWin);

    int index_google[2] = { 0 };
    int index_baidu[20] = { 0 };

    printDebugInfo();

    /*索引分隔符，存于index_xx[]*/
    if ( shmaddr_google[0] != ERRCHAR)
        getIndex(index_google, shmaddr_google);

    //if ( shmaddr_baidu[0] !=ERRCHAR )
    getIndex(index_baidu, shmaddr_baidu);

    for (int i=0; i<14; i++) {
        if (index_baidu[i] == 0)
            break;
        printf("%d ", index_baidu[i]);
    }
    printf("\n");

    /*创建layout用于显示背景图片,以及放置文本*/
    GtkWidget * layout = gtk_layout_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(newWin), layout);

    /*新建box容纳文字显示*/
    GtkWidget *vbox;
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    /*建立文字显示区域*/
    GtkWidget *view;
    GtkTextBuffer *buf;
    GtkTextIter iter;

    /*GtkTextView需要放在box里，不然弹出窗口时不显示*/
    view = gtk_text_view_new();

    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);

    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_view_set_buffer((GtkTextView*)view, buf);
    gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 20);
    gtk_layout_put(GTK_LAYOUT(layout), vbox, 30, -10);
    //gtk_text_view_set_justification ((GtkTextView*)view, GTK_JUSTIFY_FILL);

    /*设置字体相关属性*/
    setFontProperties(buf, &iter);

    /*百度翻译结果处理*/
    if ( baidu_result[0] == NULL)
        initMemory();

    /*从共享内存数据流中分离相关数据到baidu_result相关功能内存区域*/
    separateData(index_baidu, 28);


    /*来个黄金比例的矩形*/
    double width=310.0, height = width * 0.618;

    /*宽度为20时差不多是长310，由此得出此公式*/
    width = (width / 18) * maxlen_baidu;
    //height = width * 0.618;

    if ( NumZhTranFlag == 1 && NumEnTranFlag == 0 && PhoneticFlag ==0\
            && OtherWordFormFlag == 0 && lines > 2)
        height = ( lines+3 ) * 10;
    else
        height = ( lines+3 ) * 22;

    //if ( lines <= 7 && height >= 270)
        //height = 260;

    if ( width < 150 ) {
        width = 250;
        height = 140;
    }

    printf("max len of result = %d\n", maxlen_baidu);
    printf("lines=%d width=%lf height=%lf \n", lines, width, height);

    gtk_window_set_default_size(GTK_WINDOW(newWin), width, height);

    /*
     * 调整字符串
     * */
    char *p[3] ={ NULL };
    p[0] = &shmaddr_google[ACTUALSTART];
    p[1] = &shmaddr_google[index_google[0]];
    p[2] = &shmaddr_google[index_google[1]];

    char result[4096] = { '\0' };
    char explain[4096] = { '\0' };
    char related[4096] = { '\0' };

    char *storage[3] = { NULL };
    storage[0] = result;
    storage[1] = explain;
    storage[2] = related;

    if ( shmaddr_google[0]  != ERRCHAR )
        /*主要完成步骤:加入回车符使单行句子不至于太长*/
        adjustStr(p, 28, storage);
    else  {
        shmaddr_google[0] = CLEAR;
        strcpy(storage[0], "翻译超时或出现其他错误");
    }

    /*显示百度翻译结果*/
    displayBaiduTrans(buf, &iter);

    /*创建按钮*/
    GtkWidget *button = gtk_button_new();
    GdkPixbuf *src = gdk_pixbuf_new_from_file("/home/rease/.stran/Switch.png", NULL);
    GdkPixbuf *dst = gdk_pixbuf_scale_simple(src, 20, 20, GDK_INTERP_BILINEAR);
    GtkWidget *image = gtk_image_new_from_pixbuf(dst);

    g_object_unref(src);
    g_object_unref(dst);

    gtk_button_set_image(GTK_BUTTON(button), image);

    gtk_layout_put(GTK_LAYOUT(layout), button, width-50, height-45);

    struct GtkText gt;
    gt.buf = buf;
    gt.iter = &iter;
    gt.index_google[0] = index_google[0];
    gt.index_google[1] = index_google[1];
    gt.storage = storage;

    g_signal_connect(button, "clicked", G_CALLBACK(change_display), (void*)&gt);

    //gtk_widget_set_opacity(image, 0.7);
    //gtk_widget_set_opacity(view, 1);
    //gtk_widget_set_opacity(vbox, 0.4);

    /*显示*/
    gtk_widget_show_all(newWin);
    gtk_main();

    pthread_exit(NULL);
}

int destroy_newwin(GtkWidget *window) {

    maxlen_baidu = 0;

    memset(shmaddr_google, '0', 10);
    memset(shmaddr_baidu, '0', 10);

    memset(shmaddr_google, '\0', SHMSIZE-10);
    memset(shmaddr_baidu, '\0', SHMSIZE-10);

    for ( int i=0; i<BAIDUSIZE; i++ )
        memset( baidu_result[i], '\0', SHMSIZE / BAIDUSIZE );

    /*清除相关标记*/
    shmaddr_baidu[0] = CLEAR;
    shmaddr_google[0] = CLEAR;

    //gtk_window_close(GTK_WINDOW(window));
    gtk_widget_destroy(window);
    gtk_main_quit();

    /* 按了exit键后变成了单击事件，此时再双击会导致检测错误
     * 应手动置0*/
    action = 0;
    InNewWin = 0;
    CanNewEntry = 0;

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
            if ( addr != shmaddr_baidu && charNum >= 2 ) /*截取到第三个分隔符*/ {
                printf("已完成分割字符索引\n");
                break;
            }

            index[charNum++] = i + 1; /*记录字符串下标*/
        }
        p++; i++;
    }
    addr[0] = '\0';
    *(p+1) = '\0';
}


int waitForContinue() {

    int flag = 0;
    int time = 0;

    /*等待python端的翻译数据全部写入共享内存*/
    //while( shmaddr_google[0] == 0 && shmaddr_baidu[0] == 0 ) {
    while( shmaddr_google[0] != FINFLAG || shmaddr_baidu[0] != FINFLAG ) {

        if ( flag ) {
            flag = 0;
            printf("准备接收共享内存数据...\n");
        }

        /*长时间未检测到共享内存里的数据进行双击或者单机取消本次窗口显示*/
        if ( action == DOUBLECLICK/* || action == SINGLECLICK*/) {

            printf("捕获双击退出: In newWindow.c\n");

            /*此处action只代表取消显示，应重置action*/
            action = 0;
            return 1;
        }
        if ( shmaddr_google[0] == ERRCHAR) {
            printf("翻译过程出现错误\n");
            printf("准备窗口错误提示\n");
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
            CanNewEntry = 0;
            return 1;
        }
        usleep(400000);
        time++;
        if ( time >= 2 ) {
            shmaddr_google[0] = ERRCHAR;
            shmaddr_baidu[0] = ERRCHAR;
            break;
        }
    }

    return 0;
}

void initMemory() {

    if (baidu_result[0] != NULL)
        return;

    for (int i=0; i<BAIDUSIZE; i++) {

        baidu_result[i] = calloc(SHMSIZE / BAIDUSIZE, sizeof ( char ) );
        if (baidu_result[i] == NULL)
            err_exit("Error occured when calloc memory in initMemory");
    }
}

void change_display(GtkWidget *button, gpointer *arg) {
    show = ~show;
    if ( show )
        displayBaiduTrans( ((struct GtkText*)arg)->buf,((struct GtkText*)arg)->iter);
    else 
        displayGoogleTrans(button, arg);
}

void displayGoogleTrans(GtkWidget *button, gpointer *arg) {

    //printf("显示谷歌翻译结果\n");

    GtkTextIter *iter, start, end;
    iter = ((struct GtkText*)arg)->iter;
    GtkTextBuffer *buf = ((struct GtkText*)arg)->buf;

    gtk_text_buffer_get_start_iter(buf, &start);
    gtk_text_buffer_get_end_iter(buf, &end);

    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);

    char result[4096] = { '\0' };
    char explain[4096] = { '\0' };
    char related[4096] = { '\0' };

    char *storage[3] = { NULL };
    storage[0] = result;
    storage[1] = explain;
    storage[2] = related;

    int index_google[2] = { 0 };

    //printf("\n比较字符串是否相等\n\n");

    /* 比较字符串是否相等,如果不相等，说明用于谷歌翻译结果存储的共享内存被改写了，
     * 需要重新分离调整字符串*/
    if (strcmp ( &shmaddr_google[ACTUALSTART], ((struct GtkText*)arg)->storage[0] ) != 0) {

        index_google[0] = index_google[1] = 0;
        getIndex(index_google, shmaddr_google);

        char *p[3] ={ NULL };
        p[0] = &shmaddr_google[ACTUALSTART];
        p[1] = &shmaddr_google[index_google[0]];
        p[2] = &shmaddr_google[index_google[1]];

        adjustStr(p, 28, storage);

    } else {
        /*若字符串依旧相等，直接拿来用就行*/
        storage[0] = ((struct GtkText*)arg)->storage[0] ;
        storage[1] = ((struct GtkText*)arg)->storage[1] ;
        storage[2] = ((struct GtkText*)arg)->storage[2] ;
    }

    char doubleEnter[] = "\n\n";
    char enter[] = "\n";

    /*插入输入原文*/
    if ( strlen( SourceInput )  < 30 )
        gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[0], -1, 
                "black-font", "gray_background", "bold-style", "Uneditable", "font-size-14", NULL);

    /*插入翻译结果*/
    for ( int i=0; i<3; i++ ) {

        //printf("storage[]=%s\n", storage[i]);
        if ( storage[i][0] != '\0') {

            gtk_text_buffer_insert_with_tags_by_name(buf, iter, storage[i], -1, 
                    "green-font", "gray_background", "bold-style", "Uneditable", "font-size-11", NULL);

            if ( i<2 && storage[i+1][0] != '\0')
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, doubleEnter, -1, NULL, NULL);
            else 
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
        }
    }
}
void displayBaiduTrans(GtkTextBuffer *buf, GtkTextIter* iter) {

    //printf("显示百度翻译结果\n");

    GtkTextIter start, end;

    /*找到开头和结尾并删除，重新定位到初始为位置0*/
    gtk_text_buffer_get_end_iter(buf, &end);
    gtk_text_buffer_get_start_iter(buf, &start);

    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);

    char enter[] = "\n";

    for ( int i=0; i<BAIDUSIZE-1; i++ ) {

        if ( baidu_result[i][0] != '\0') {

            if ( i == 0 && strlen(baidu_result[i]) < 30 )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i], -1, 
                        "black-font", "gray_background", "bold-style", "Uneditable", "font-size-15", "letter-spacing","underline", \
                         NULL);
            else if ( i == 1 )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i], -1, 
                        "blue-font", "gray_background", "heavy-font", "Uneditable", "font-size-11", "letter-spacing", NULL);
            else if ( i == 4 )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i], -1, 
                        "brown-font", "gray_background", "heavy-font", "Uneditable", "font-size-11","letter-spacing", NULL);
            else if ( i != 0 )
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, baidu_result[i], -1, 
                        "green-font", "gray_background", "heavy-font", "Uneditable", "font-size-11", "letter-spacing", NULL);


            if ( 0 && i == 0 && ( strlen(baidu_result[3]) ==0 && strlen(baidu_result[4]) == 0\
                        && strlen(baidu_result[2]) != 0 && strlen(baidu_result[1]) == 0)) {


                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
                //printf("回车1\n");
            }

            else if ( i == 1 && strlen(baidu_result[1]) != 0 && ( strlen(baidu_result[2]) != 0\
                        || strlen(baidu_result[3]) != 0 || strlen(baidu_result[4]) != 0)) {

                //printf("回车2\n");
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
            }

            else if ( i == 2 && ( strlen(baidu_result[3]) != 0 || strlen(baidu_result[4]) != 0 ) ) {
                //printf("回车3\n");
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
            }


            else if ( i == 3 && ( strlen(baidu_result[4]) != 0) )  {

                //printf("回车4\n");
                gtk_text_buffer_insert_with_tags_by_name(buf, iter, enter, -1, NULL, NULL);
            }
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
    gtk_text_buffer_create_tag(buf, "font-size-27", "font", "27", NULL );
    gtk_text_buffer_create_tag(buf, "font-size-17", "font", "17", NULL );
    gtk_text_buffer_create_tag(buf, "font-size-14", "font", "14", NULL );
    gtk_text_buffer_create_tag(buf, "font-size-15", "font", "15", NULL );
    gtk_text_buffer_create_tag(buf, "font-size-13", "font", "13", NULL );
    gtk_text_buffer_create_tag(buf, "font-size-12", "font", "12", NULL );
    gtk_text_buffer_create_tag(buf, "gray_background", "background", "#ffffff", NULL);
    gtk_text_buffer_create_tag(buf, "letter-spacing", "letter-spacing", 100, NULL);
    gtk_text_buffer_create_tag(buf, "underline", "underline", PANGO_UNDERLINE_SINGLE, NULL);
    //gtk_text_buffer_create_tag(buf, "wrap-mode", "wrap", GTK_WRAP_WORD_CHAR , NULL);

    gtk_text_buffer_get_iter_at_offset(buf, iter, 0);
}

void printDebugInfo() {

    printf("百度翻译结果: %s\n", &shmaddr_baidu[ACTUALSTART]);
    printf("\nFinish标志位: %c", shmaddr_baidu[0]);
    printf("\nPhonetic标志位: %c", shmaddr_baidu[1]);
    printf("\nNumbers of zhTrans标志位: %c", shmaddr_baidu[2]);
    printf("\nNumbers of enTrans标志位: %c", shmaddr_baidu[3]);
    printf("\nOther forms of word标志位: %c", shmaddr_baidu[4]);
    printf("\nNumbers of audio links标志位: %c\n", shmaddr_baidu[5]);

    printf("\n谷歌翻译结果: %s\n\n", &shmaddr_google[ACTUALSTART]);
}
