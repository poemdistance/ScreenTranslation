/*GuiEntrance.c
 * 程序功能:
 *  创建翻译入口图标*/

#include "common.h"
#include "windowData.h"
#include "expanduser.h"
#include "cleanup.h"
#include "configControl.h"

static int aboveWindow = 0;
static int timeout_id_1;
static int timeout_id_2;

typedef struct {

    GtkWidget *window;
    GtkWidget *button;
    CommunicationData *md;
    ShmData *sd;
    Arg *arg;

}ClickData;

int quit_entry(void *arg);
int quit_test(void *arg);
void setNewWinFlag(GtkWidget *button, ClickData *cd );
void leave_event();
void enter_event();

void *GuiEntrance(void *arg) {

    pbblue ( "启动线程GuiEntrance" );

    /*添加超时和单击销毁图标回调函数*/
    ClickData cd;
    ConfigData *config = ((Arg*)arg)->cd;
    CommunicationData *md = ((Arg*)arg)->md;
    ShmData *sd = ((Arg*)arg)->sd;
    cd.md = md;
    cd.sd = sd;
    cd.arg = arg;

    aboveWindow = 0;

    /*等待鼠标事件到来创建入口图标*/
    while(1) {

        if ( md->canNewEntrance ) {
            pmag ( "创建入口图标" );
            break;
        }

        if ( md->sigtermNotify ) 
            return NULL;

        usleep(500000);
    }


    GtkWidget *window;

    int iconOffsetX = config->iconOffsetX;
    int iconOffsetY = config->iconOffsetY;

    pbmag ( "Icon offset: %d %d", config->iconOffsetX, config->iconOffsetY );

    /*入口图标销毁标志置0，表示处于显示状态*/
    md->iconShowing = 1;
    md->canNewEntrance = 0;

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);

    /*设置窗口基本属性*/
    gtk_window_set_title(GTK_WINDOW(window), "");
    gtk_window_set_default_size(GTK_WINDOW(window), 15,15);
    gtk_window_set_deletable(GTK_WINDOW(window), FALSE); 
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_TOOLBAR); 
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE); 
    GtkWidget *button = gtk_button_new();


    /*TODO:添加文件存在性检测*/
    /*添加图标*/

    GdkPixbuf *src = gdk_pixbuf_new_from_file(expanduser("/home/$USER/.stran/tran.png"), NULL);
    GdkPixbuf *dst = gdk_pixbuf_scale_simple(src, 30, 30, GDK_INTERP_BILINEAR);
    GtkWidget *image = gtk_image_new_from_pixbuf(dst);

    g_object_unref(src);
    g_object_unref(dst);

    //GtkWidget *image = gtk_image_new_from_file("/home/usernamee/.stran/tran.png");
    gtk_button_set_image(GTK_BUTTON(button), image);
    gtk_container_add(GTK_CONTAINER(window), button);
    gtk_widget_set_app_paintable(window, TRUE);
    gtk_widget_set_opacity(window, 0.7);
    GdkScreen *screen = gtk_widget_get_screen(window);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
    gtk_widget_set_visual(window, visual);

    cd.window = window;
    cd.button = button;

    /*移动入口图标防止遮挡视线*/
    gint cx, cy;
    gtk_window_get_position(GTK_WINDOW(window), &cx, &cy);

    gtk_window_move(
            GTK_WINDOW(window), 
            cx+iconOffsetX, 
            cy+iconOffsetY
            );

    gtk_widget_show_all(window);

    timeout_id_1 = g_timeout_add(
            config->iconShowTime == 0 ? 1200 : config->iconShowTime,
            quit_entry,
            &cd);

    timeout_id_2 = g_timeout_add(100, quit_test, &cd);

    /*连接鼠标点击事件*/
    g_signal_connect(button, "clicked",G_CALLBACK(setNewWinFlag), &cd);

    /*监听鼠标进入和离开窗口事件*/
    g_signal_connect(GTK_BUTTON(button), "leave", G_CALLBACK(leave_event), NULL);
    g_signal_connect(GTK_BUTTON(button), "enter", G_CALLBACK(enter_event), NULL);


    gtk_main();

    pthread_exit(NULL);
}

void setNewWinFlag(GtkWidget *button, ClickData *cd ) {

    pbmag ( "------------- 点击入口图标 -------------" );

    GtkWidget *window = cd->window;
    CommunicationData *md = cd->md;
    md->canNewWin = 1;
    md->iconShowing = 0;

    /*退出时注意注销超时回调函数，否则下一次创建的
     * 入口图标可能刚好创建就超时导致不显示*/
    g_source_remove(timeout_id_2);
    g_source_remove(timeout_id_1);

    gtk_widget_destroy(button);
    gtk_widget_destroy(window);
    gtk_main_quit();
}

int quit_test(void *arg) {

    ClickData *cd = (ClickData*)arg;
    CommunicationData *md = cd->md;
    GtkWidget *button = cd->button;
    GtkWidget *window = cd->window;

    /*入口图标已在quit_entry中销毁,返回FALSE不再调用此函数*/
    if ( !md->iconShowing )
        return FALSE;

    /*不在窗口上的单击定义为销毁窗口命令*/
    if ( md->iconShowing && !aboveWindow ) {

        if ( md->destroyIcon && !aboveWindow) {

            printf("GuiEntrance: 单击销毁\n");

            clearMemory ( cd->arg );

            md->canNewWin = 0;
            md->destroyIcon = 0;
            md->iconShowing = 0;

            g_source_remove(timeout_id_1);
            g_source_remove(timeout_id_2);

            gtk_widget_destroy(button);
            gtk_widget_destroy(window);
            gtk_main_quit();

            return FALSE;
        }
    }

    return TRUE;
}

int quit_entry(void *arg) {

    ClickData *cd = (ClickData*)arg;
    CommunicationData *md = cd->md;
    GtkWidget *button = cd->button;
    GtkWidget *window = cd->window;

    /*入口图标已经在quit_test中销毁时返回FALSE不再调用*/
    if ( !md->iconShowing )
        return FALSE;

    if ( button &&  window &&  md->iconShowing && !aboveWindow) {

        printf("GuiEntrance: 超时销毁\n");

        clearMemory ( cd->arg );

        md->canNewWin = 0;
        md->iconShowing = 0;

        g_source_remove(timeout_id_1);
        g_source_remove(timeout_id_2);
        gtk_widget_destroy(button);
        gtk_widget_destroy(window);
        gtk_main_quit();
        return FALSE;
    }

    return TRUE;
}

void leave_event() {
    printf("Leaved window\n");
    aboveWindow = 0;
}

void enter_event() {
    printf("Enter window\n");
    aboveWindow = 1;
}
