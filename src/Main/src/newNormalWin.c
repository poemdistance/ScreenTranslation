#include <gtk/gtk.h>
#include <ctype.h>
#include "common.h"
#include "newWindow.h"
#include "audio.h"
#include "cleanup.h"
#include "expanduser.h"
#include "dataStatistics.h"
#include "memoryControl.h"
#include "windowData.h"
#include "pointer.h"
#include "configControl.h"

#define BOLD_TYPE (1)
#define NOT_BOLD (0)
#define TRANSPARENT (0)
#define NOT_TRANSPARENT (1)
#define SHADOW_BORDER_WIDTH (17)
#define SHADOW_BORDER_HEIGTH (17)

#define UI_SRC ("/home/$USER/.stran/sure.ui")

typedef void (*Display_func)(GtkWidget *, gpointer *);

char **baidu_result[BAIDUSIZE] = { NULL };
char *google_result[GOOGLESIZE] = { NULL };
char **mysql_result[MYSQLSIZE] = { NULL };
char *tmp;

/* 用于和detectMouse通信，当已经新建翻译结果显示窗口时，
 * 不再检测鼠标动作*/
volatile sig_atomic_t InNewWin = 0;

/* 鼠标动作标志位*/
extern volatile sig_atomic_t action;
extern volatile sig_atomic_t CanNewWin;
static int focustimes = 0;
int timeout_id = 0;
int movewindow_timeout_id = 0;
static int moveDone = 1;
gboolean resizeAction = FALSE;

GtkWidget *setWidgetProperties ( GtkWidget *widget, double fontSizeScale ,
        const char *rgb, int bold, int alpha);
GtkWidget *addUnderline ( GtkWidget *widget, const char *rgb, guint type);

void clearContentListBox ( GtkWidget *listbox  );
void getPosTran ( char *src, char **pos, char **tran, char split );
void on_tran_button_clicked_cb ( GtkWidget *button, WinData *wd);
void reHideWidget ( GtkWidget **widgets, int len );
gboolean on_motion_notify_event ( GtkWidget *widget, GdkEventButton *event, WinData *wd);
GtkWidget *addUnderline ( GtkWidget *widget, const char *rgb, guint type);
void on_win_size_allocate_cb ( GtkWidget *window, GdkRectangle *alloc, WinData *wd);
static gboolean expose_draw(GtkWidget *widget, GdkEventExpose *event, WinData *wd );
void on_maximize_button_clicked_cb ( GtkWidget *button, WinData *wd);
int focus_request(void *data);

enum {
    NORTH_AREA,
    NORTH_EAST_AREA,
    EAST_AREA,
    SOUTH_EAST_AREA,
    SOUTH_AREA,
    SOUTH_WEST_AREA,
    WEST_AREA,
    NORTH_WEST_AREA,
};

static inline int previousWindow ( int who ) {
    return who > 1 ? who -1 : 3;
}

static inline Display_func choice_display( int who ) {

    return ( who == BAIDU ? displayBaiduTrans : \
            ( who == GOOGLE ? displayGoogleTrans :\
              displayOfflineTrans) );
}

void selectDisplay( WinData *wd ) {

    if ( wd->gotBaiduTran )
        wd->who = BAIDU;
    else if ( wd->gotOfflineTran )
        wd->who = MYSQL;
    else
        wd->who = GOOGLE;

    GtkWidget *pressButton = GET_BUTTON ( wd, wd->who );
    on_tran_button_clicked_cb ( pressButton, wd );
}

void show_all_visible_widget ( WinData *wd ) {

    gtk_widget_show_all ( wd->window );
    reHideWidget(wd->needToBeHiddenWidget, 
            sizeof(wd->needToBeHiddenWidget)/sizeof(GtkWidget*));
}

int adjustTargetPosition( 
        int *targetx, int *targety,
        int pointerx, int pointery,
        WinData *wd ) {

    ConfigData *cd = wd->cd;
    GdkRectangle workarea = { '\0' };

    show_all_visible_widget ( wd );

    gdk_monitor_get_workarea(
            gdk_display_get_monitor_at_window ( 
                gdk_window_get_display(gtk_widget_get_window(wd->window)),
                /* gdk_display_get_default(), */
                gtk_widget_get_window(wd->window)),
            &workarea);

    if ( cd->iconOffsetY < 0 && *targety < 0 ) {
        *targety = pointery - cd->iconOffsetY;
        pcyan ( "重新调整y轴位置 y=%d", *targety);
    }
    else if ( cd->iconOffsetY > 0 && *targety+wd->height > workarea.height ) {
        *targety = pointery - wd->height - cd->iconOffsetY;
        pcyan ( "重新调整y轴位置 y=%d", *targety);
    }

    return 0;
}

void on_win_size_allocate_cb (
        GtkWidget *window,
        GdkRectangle *alloc,
        WinData *wd
        ) {

    void handleShadowBorder ( WinData *wd );
    handleShadowBorder ( wd );

    if ( wd->time >= 100 )  {
        /* pred ( "超时不再检测窗口大小变化" ); */
        return;
    }

    pcyan ( "Win Size Allocate: %d %d %d %d",
            alloc->x, alloc->y, alloc->width, alloc->height );

    if ( wd->doubleClickAction ) {
        pred ( "Double click action , will not send notify to move window" );
        return;
    }

    wd->width = alloc->width;
    wd->height = alloc->height;

    if ( wd->width > wd->previousWidth || wd->height > wd->previousHeight ) {
        pcyan ( "Move window notify" );
        wd->moveWindowNotify = TRUE;
    }

    wd->previousWidth = wd->width;
    wd->previousHeight = wd->height;
}

/* 
 * (x0, y0): 窗口左上角坐标(不可见部分)
 * (x1, y1): 窗口右下角坐标(亦不可见)
 * */
static inline gboolean isPointerInOurWin 
             ( int x0, int y0, int x1, int y1, WinData *wd ) {

                 return 
                     wd->cd->pointerx >= x0 && 
                     wd->cd->pointery >= y0 &&
                     wd->cd->pointerx <= x1 &&
                     wd->cd->pointery <= y1;
             }

/* 
 * (x0, y0): 窗口左上角坐标(可见部分)
 * (x1, y1): 窗口右下角坐标(可见部分)
 * */
    static inline gboolean 
             isPointerInOurVisibleWin ( int x0, int y0, int x1, int y1, WinData *wd ) 
{

    return isPointerInOurWin ( x0, y0, x1, y1, wd );
}

int check_pointer_and_window_position ( void *data ) {


    WinData *wd = data;
    ConfigData *cd = wd->cd;

    int invisible_win_root_x = 0;
    int invisible_win_root_y = 0;
    int invisible_win_width = 0;
    int invisible_win_height = 0;
    int invisible_right_down_x = 0;
    int invisible_right_down_y = 0;
    int border_width = 0;
    int border_height = 0;
    int visible_win_root_x = 0;
    int visible_win_root_y = 0;
    int visible_right_down_x = 0;
    int visible_right_down_y = 0;
    int visible_win_width = 0;
    int visible_win_height = 0;

    GdkWindow *win = gtk_widget_get_window ( wd->window );

    /* 此函数获取真实窗口大小, 比如使用了decoration，其返回的
     * 窗口坐标和宽高包含了不可见部分, 当前窗口设计屏蔽了默认
     * 的装饰，标题栏，周围的阴影效果，以及Resize功能皆有项目
     * 代替完成*/
    gdk_window_get_geometry ( win,
            &invisible_win_root_x, &invisible_win_root_y,
            &invisible_win_width, &invisible_win_height );

    /* 由于禁用了decoration, 代替以项目完成，所以这里获取的窗口大小值也会
     * 包含不可见部分, 因为不可见部分现在来说本就是实际窗口控制的一部分，
     * 只是被我们隐藏了,而原来使用decoration时，周围不可见部分不是由我们完成的.*/
    /* int win_width = 0; */
    /* int win_height = 0; */
    /* gtk_window_get_size ( GTK_WINDOW(wd->window), &win_width, &win_height ); */

    border_width = SHADOW_BORDER_WIDTH;
    border_height = SHADOW_BORDER_HEIGTH;

    visible_win_root_x = invisible_win_root_x + border_width;
    visible_win_root_y = invisible_win_root_y + border_height;
    visible_win_width = invisible_win_width - 2 * border_width;
    visible_win_height = invisible_win_height - 2 * border_height;

    visible_right_down_x = visible_win_root_x + visible_win_height  ;
    visible_right_down_y = visible_win_root_y + visible_win_width  ;

    invisible_right_down_x = invisible_win_root_x + invisible_win_width;
    invisible_right_down_y = invisible_win_root_y + invisible_win_height;

    if ( ! isPointerInOurWin ( 
                invisible_win_root_x, invisible_win_root_y,
                invisible_right_down_x, invisible_right_down_y, 
                wd )) {

        /* resizeAction = FALSE; */
    }

    if ( isPointerInOurVisibleWin (
                visible_win_root_x, visible_win_root_y,
                visible_right_down_x, visible_right_down_y, 
                wd) ) {
    }

    if ( ! wd->moveWindowNotify ) {
        /* pred ( "未检测到窗口移动通知" ); */
        return TRUE;
    }

    wd->moveWindowNotify = FALSE;

    pcyan ( "Window Invisible Right Down Position: %d %d Pointer Position: %d %d",
            invisible_right_down_x, invisible_right_down_y, cd->pointerx, cd->pointery );

    int targetX = cd->pointerx -
        ( cd->pointerOffsetX *1.0 / 400 ) * invisible_win_width;;
    int targetY = cd->pointery - 
        ( cd->pointerOffsetY *1.0 / 252 ) * invisible_win_height;

    if ( ! wd->quickSearchFlag ) {

        if ( cd->allowAutoAdjust )
            adjustTargetPosition( &targetX, &targetY, cd->pointerx, cd->pointery, wd );

        pbcyan ( "Move window, Win size: %d %d", wd->width, wd->height );
        gdk_window_move ( 
                gtk_widget_get_window(wd->window),
                targetX, targetY );

        moveDone = 1;
        wd->showLock = FALSE;
    }

    wd->previousWidth = invisible_win_width;
    wd->previousHeight = invisible_win_height;

    return TRUE;
}


/* 本函数代码借鉴自xdotool部分源码*/
int focusOurWindow( WinData *wd ) {

    /* ConfigData *cd = wd->cd; */

    pgreen ( "Focus request for translation window" );

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

gboolean on_button_press_cb ( 
        GtkWidget *widget,
        GdkEventButton *event,
        WinData *wd) {

    pbred ( "Button press:%d %d", event->type, event->state );

    if ( event->type == GDK_2BUTTON_PRESS ) {

        pmag ( "双击切换窗口大小" );
        wd->doubleClickAction = TRUE;;
        on_maximize_button_clicked_cb ( NULL, wd );
    }

    if (event->type == GDK_BUTTON_PRESS  && event->state == 16 )
    {
        if (event->button == 1) {
            gtk_window_begin_move_drag(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                    event->button,
                    event->x_root,
                    event->y_root,
                    event->time);
        }
        wd->beginDrag = TRUE;
    }

    wd->mousePress = TRUE;
    return FALSE;
}

int detect_outside_click_action ( void *data ) {

    WinData *wd = data;
    ConfigData *cd = WINDATA(data)->cd;
    static gboolean block = FALSE;
    static int lock = 1;
    static int reCheck = 0;
    static int tick = 0;

    if ( wd->time <= 100 ) wd->time++;

    /* 窗口内双击动作被设计为最大化或者取消最大化窗口
     * ，检测到此动作后doubleClickAciton置位，在计数100
     * 次后恢复默认值FALSE，防止双击动作被误测为区域外点击
     * 事件而将窗口关闭*/
    if ( wd->doubleClickAction && ++tick == 100 ) {
        wd->doubleClickAction = FALSE;
        moveDone = 0;
        tick = 0;
    }

    if ( cd->buttonState == BUTTON_RELEASE ) {
        wd->beginDrag = FALSE;
        block = FALSE;
        resizeAction = FALSE;
    }

    /* 请先阅读下一块的注释.
     * 重新检测设置窗口的关闭状态，如果设置窗口已经关闭，则重新使能
     * 翻译窗口的置顶*/
    if ( reCheck && wd->shmaddr_setting[1] == '0' ) {
        pcyan ( "Set keep above: true" );
        gtk_window_set_keep_above ( GTK_WINDOW(wd->window), TRUE );
        reCheck = 0;
    }

    /* 设置窗口打开的时候禁止翻译窗口置顶，但是不能一直执行禁止置顶的语句
     * 不然设置无效，所以通过将openSettingWindowAction赋值false实现只执行
     * 一次取消置顶，取消以后使能reCheck, 当设置窗口关闭的时候再使能翻译
     * 窗口的置顶.*/
    if ( wd->openSettingWindowAction ){
        pcyan ( "Set keep above: false" );
        gtk_window_set_keep_above ( GTK_WINDOW(wd->window), FALSE );
        wd->openSettingWindowAction = FALSE;
        reCheck = 1;
    }

    if ( wd->pinEnable ) return TRUE;

    if ( !wd->showLock && lock && (moveDone || wd->quickSearchFlag) ) {
        printf("Show all widget\n");
        show_all_visible_widget ( wd );
        focus_request((void*)wd);
        wd->showLock = TRUE;

        /* Quick Search 弹出的窗口放到屏幕中央，
         * 之后只进行一次聚焦请求，而且不移动窗口，
         * 所以需要锁住这里的逻辑，防止不断调用
         * gtk_wiget_show_all()*/
        if ( wd->quickSearchFlag ) lock = 0;
    }

    if ( ! action || action == SLIDE ) return TRUE; 

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
    if ( block && cd->buttonState == BUTTON_PRESS ) { 
        return TRUE;
    }

    block = FALSE;

    gint wx = 0;
    gint wy = 0;

    gtk_window_get_position ( 
            GTK_WINDOW(wd->window), &wx, &wy);

    gint pointerX = cd->pointerx;
    gint pointerY = cd->pointery;
    gint w = 0;
    gint h = 0;
    gtk_window_get_size ( GTK_WINDOW(wd->window), &w, &h );

    gboolean condition = FALSE;
    condition = 
        pointerX >= wx && pointerX <= wx+w &&
        pointerY >= wy && pointerY <= wy+h;

    if ( condition && cd->buttonState == BUTTON_PRESS ) {
        pgreen ( "Block -> TRUE" );
        block = TRUE;
        action = 0;
        return TRUE;
    }

    if ( action == SINGLE_CLICK && cd->buttonState == BUTTON_RELEASE)  {
        /* printf("begignDrag and resizeAction to False\n"); */
        wd->beginDrag = FALSE;
        resizeAction = FALSE;
    }

    if ( 
            ! condition  && 
            ! resizeAction && 
            ! wd->beginDrag &&
            ! wd->doubleClickAction) {

        pbmag ( "区域外点击销毁窗口. Action=%d", action );
        destroyNormalWin ( NULL, wd );
        return FALSE;
    }

    return TRUE;
}

int focus_request(void *data) {

    WinData *wd = data;

    /* 每隔一定时间多次尝试重新聚焦窗口防止聚焦窗口被抢占*/
    if ( focustimes <= 5 ) {
        focusOurWindow ( wd );
        focustimes++;
    }

    if ( focustimes > 5 ) {

        g_source_remove ( timeout_id );

        timeout_id = g_timeout_add (
                10,
                detect_outside_click_action,
                wd);
    }

    return TRUE;
}

void addToHiddenArray( GtkWidget *widget, WinData *wd ) {

    for ( int i=0; i<sizeof(wd->needToBeHiddenWidget)/sizeof(GtkWidget*); i++ ) {

        if ( wd->needToBeHiddenWidget[i] == widget ) 
            return;

        if ( ! wd->needToBeHiddenWidget[i] ) {
            wd->needToBeHiddenWidget[i] = widget;
            return;
        }
    }
}

void dropFromHiddenArray ( GtkWidget *widget, WinData *wd ) {

    int found = 0;
    int i = 0;
    int k = 0;

    for ( i=0; i<sizeof(wd->needToBeHiddenWidget)/sizeof(GtkWidget*); i++ ) {

        if ( wd->needToBeHiddenWidget[i] == widget ) {
            found = 1;
            wd->needToBeHiddenWidget[i] = (void*)-1;
            break;
        }
    }
    if ( ! found ) return;

    for ( i=0, k=0; i<sizeof(wd->needToBeHiddenWidget)/sizeof(GtkWidget*); i++ ) {

        if ( wd->needToBeHiddenWidget[i] != (void*)-1 ) {
            wd->needToBeHiddenWidget[k++] = wd->needToBeHiddenWidget[i];
            continue;
        }

        if ( !wd->needToBeHiddenWidget[i] ) break;
    }

    wd->needToBeHiddenWidget[i-1] = NULL;
}

void reHideWidget ( GtkWidget **widgets, int len ) {

    int i=0;
    for ( i=0; i<len && widgets[i]; i++ )
        gtk_widget_hide ( widgets[i] );
}

void on_pin_button_clicked_cb (
        GtkWidget *button,
        WinData *wd
        ) {

    static int mode = 0;
    mode = ~mode;
    wd->pinEnable = !wd->pinEnable;
    if ( mode ) {
        wd->pinEnable = TRUE;
        gtk_widget_show ( wd->selectedPin );
        dropFromHiddenArray( wd->selectedPin, wd );
    }
    else {
        pgreen ( "Pin Disable" );
        gtk_widget_show ( wd->unselectedPin );
        dropFromHiddenArray(wd->unselectedPin, wd);
    }

    gtk_widget_hide ( button );
    addToHiddenArray ( button, wd );
}

int getIdByButton ( WinData *wd, GtkWidget *button ) {

    if ( button == wd->baiduButton || button == wd->selectedBing )
        return BAIDU;
    else if ( button == wd->mysqlButton || button == wd->selectedOffline )
        return OFFLINE;
    else if ( button == wd->googleButton || button == wd->selectedGoogle )
        return GOOGLE;

    return -1;
}

/* 点击翻译源等按钮时更换显示的图片以指示当前
 * 点击的按钮和所在页面(点击后更换为左下角有点
 * 的图片)*/
void on_tran_button_clicked_cb (

        GtkWidget *button,
        WinData *wd) {

    int i = 0;
    int found = 0;
    for ( i=0; i<sizeof(wd->unselectedButton)/sizeof(GtkWidget*); i++ ) {
        if ( wd->unselectedButton[i] == wd->unselectedPin ) continue;
        if ( button == wd->unselectedButton[i] ) {
            found = 1; break;
        }
    }

    for ( i=0; found && i<sizeof(wd->unselectedButton)/sizeof(GtkWidget*); i++ ) {

        if ( wd->unselectedButton[i] == wd->unselectedPin ) continue;

        if ( button == wd->unselectedButton[i] ) {

            gtk_widget_show ( wd->selectedButton[i] );
            gtk_widget_hide ( wd->unselectedButton[i] );

            dropFromHiddenArray(wd->selectedButton[i], wd);
            addToHiddenArray(wd->unselectedButton[i], wd);

            continue;
        }
        gtk_widget_show ( wd->unselectedButton[i] );
        gtk_widget_hide ( wd->selectedButton[i] );

        dropFromHiddenArray(wd->unselectedButton[i], wd);
        addToHiddenArray(wd->selectedButton[i], wd);
    }

    Display_func func = choice_display ( getIdByButton(wd,button) );
    func ( button, (void*)wd );
}


void loadSelectedButton( WinData *wd ) {

    GtkWidget *googleButtonSelected;
    GtkWidget *bingButtonSelected;
    GtkWidget *offlineButtonSelected;
    GtkWidget *pinButtonSelected;

    GtkBuilder *builder = gtk_builder_new_from_file(expanduser(UI_SRC));

    googleButtonSelected =
        (GtkWidget*)gtk_builder_get_object(GTK_BUILDER(builder), "google_button_selected");
    bingButtonSelected =
        (GtkWidget*)gtk_builder_get_object(GTK_BUILDER(builder), "bing_button_selected");
    offlineButtonSelected =
        (GtkWidget*)gtk_builder_get_object(GTK_BUILDER(builder), "offline_button_selected");;
    pinButtonSelected = 
        (GtkWidget*)gtk_builder_get_object(GTK_BUILDER(builder), "pin_button_selected");;

    wd->selectedGoogle = wd->selectedButton[0] = googleButtonSelected;
    wd->selectedOffline = wd->selectedButton[1] = offlineButtonSelected;
    wd->selectedBing = wd->selectedButton[2] = bingButtonSelected;
    wd->selectedPin = wd->selectedButton[3] = pinButtonSelected;

    gtk_grid_attach ( GTK_GRID(wd->ctrl_grid), googleButtonSelected, 2, 0, 1, 1 );
    gtk_grid_attach ( GTK_GRID(wd->ctrl_grid), offlineButtonSelected, 3, 0, 1, 1 );
    gtk_grid_attach ( GTK_GRID(wd->ctrl_grid), bingButtonSelected, 4, 0, 1, 1 );
    gtk_grid_attach ( GTK_GRID(wd->ctrl_grid), pinButtonSelected, 5, 0, 1, 1 );

    /* 上面按钮放置的位置跟初始加载到界面上的按钮是重合的, 所以需要进行
     * 隐藏，否则按钮无法点击*/
    for ( int i=0; i<sizeof(wd->selectedButton)/sizeof(GtkWidget*) && wd->selectedButton[i]; i++ )
        gtk_widget_hide ( wd->selectedButton[i] );

    g_signal_connect ( pinButtonSelected, "clicked", 
            G_CALLBACK(on_pin_button_clicked_cb), wd );

    for ( int i=0; i<3; i++ )
        g_signal_connect ( wd->selectedButton[i], "clicked", 
                G_CALLBACK(on_tran_button_clicked_cb), wd );
}

void disable_row_selectable_activatable ( GtkWidget *listbox ) {

    inline void disable_selectable_activatable( GtkWidget *widget, gpointer data ) {
        gtk_list_box_row_set_selectable ( GTK_LIST_BOX_ROW(widget), FALSE );
        gtk_list_box_row_set_activatable ( GTK_LIST_BOX_ROW(widget), FALSE );
    }

    gtk_container_forall ( 
            GTK_CONTAINER(listbox),
            disable_selectable_activatable,
            NULL
            );
}

int noContentLeft ( char *str ) {

    if ( !str ) return 1;

    char *p = str;
    while ( *p && ( *p == '\t' || *p == ' ' || *p == '\n' ) ) p++;

    if ( *p != '\0' )
        return 0;

    return 1;
}

void appendTranToItemListBox ( 
        GtkWidget *item_listbox,
        gchar *tran,
        gchar *color,
        WinData *wd ) {

    gchar *p = tran;
    gchar *s = tran;

    void append ( GtkWidget *item_listbox, gchar *tran, gchar *color, WinData *wd ) {

        GtkWidget *label = NULL;
        gint INSERT_END = -1;
        label = gtk_label_new ( NULL );
        addUnderline ( label, "#00aaff" , PANGO_UNDERLINE_LOW );
        setWidgetProperties ( label, 1.1, color, BOLD_TYPE, NOT_TRANSPARENT );
        gtk_label_set_text ( GTK_LABEL(label), tran );
        gtk_list_box_insert ( GTK_LIST_BOX(item_listbox), label, INSERT_END );
        gtk_widget_set_halign ( label, GTK_ALIGN_START );
        gtk_widget_set_valign ( label, GTK_ALIGN_FILL );
    }


    while ( 0 ) {
        p = strchr ( s, '\n' );
        if ( !p || noContentLeft(p) ) break;
        *p = '\0';
        append ( item_listbox, s, color, wd );
        s = p+1;
    }

    /* 去除最末尾的所有可能存在的回车符*/
    p = s;
    while ( *(p+1) ) p++;
    while ( p!=tran && (isblank(*p) || *p == '\n') ) *p--='\0';

    append ( item_listbox, s, color,  wd );
}

void on_minimize_button_clicked_cb ( 
        GtkWidget *button,
        WinData *wd) {

    gtk_window_iconify ( GTK_WINDOW(wd->window) );
}

void handleShadowBorder ( WinData *wd ) {

    if ( ! gtk_window_is_maximized ( GTK_WINDOW(wd->window) ) ) {
        gtk_widget_show ( wd->topBox );
        gtk_widget_show ( wd->bottomBox );
        gtk_widget_show ( wd->leftBox );
        gtk_widget_show ( wd->rightBox );

        gtk_widget_queue_draw ( wd->topBox );
        gtk_widget_queue_draw ( wd->bottomBox );
        gtk_widget_queue_draw ( wd->leftBox );
        gtk_widget_queue_draw ( wd->rightBox );
    }
    else {

        gtk_widget_hide ( wd->topBox );
        gtk_widget_hide ( wd->bottomBox );
        gtk_widget_hide ( wd->leftBox );
        gtk_widget_hide ( wd->rightBox );
    }
}

void on_maximize_button_clicked_cb ( 
        GtkWidget *button,
        WinData *wd) {

    if ( ! gtk_window_is_maximized ( GTK_WINDOW(wd->window) ) ) {
        gtk_window_maximize ( GTK_WINDOW(wd->window) );
        gtk_widget_hide ( wd->topBox );
        gtk_widget_hide ( wd->bottomBox );
        gtk_widget_hide ( wd->leftBox );
        gtk_widget_hide ( wd->rightBox );
    }
    else {

        gtk_window_unmaximize ( GTK_WINDOW(wd->window) );

        gtk_widget_show ( wd->topBox );
        gtk_widget_show ( wd->bottomBox );
        gtk_widget_show ( wd->leftBox );
        gtk_widget_show ( wd->rightBox );

        gtk_widget_queue_draw ( wd->topBox );
        gtk_widget_queue_draw ( wd->bottomBox );
        gtk_widget_queue_draw ( wd->leftBox );
        gtk_widget_queue_draw ( wd->rightBox );
    }

    wd->doubleClickAction = TRUE;

}

void insertTextContentBox (  gchar *pos, gchar *tran, WinData *wd, gchar *color  ) {

    GtkBuilder *builder = NULL;
    GtkWidget *item_box = NULL;
    GtkWidget *item_label = NULL;
    GtkWidget *item_listbox = NULL;
    gint INSERT_END = -1;

    builder = gtk_builder_new_from_file ( expanduser(UI_SRC) );
    item_box = (GtkWidget*)gtk_builder_get_object ( builder, "item_box" );
    item_label = (GtkWidget*)gtk_builder_get_object ( builder, "item_label" );
    item_listbox = (GtkWidget*)gtk_builder_get_object ( builder, "item_listbox" );

    gtk_label_set_text ( GTK_LABEL(item_label), pos );
    setWidgetProperties ( item_label,  1.1, color,  BOLD_TYPE, NOT_TRANSPARENT );

    /* gtk_widget_override_background_color ( item_label, 0, &wd->rgba ); */
    /* gtk_entry_set_has_frame ( GTK_ENTRY(item_label), FALSE ); */

    gtk_list_box_insert ( GTK_LIST_BOX(wd->content_listbox), item_box, INSERT_END );

    appendTranToItemListBox ( item_listbox, tran, color, wd );

    static int i = 0;
    gtk_widget_set_size_request ( wd->window, 1, i++ ); /* 触发窗口重新调整大小*/
    show_all_visible_widget ( wd );
    disable_row_selectable_activatable ( wd->content_listbox );
    disable_row_selectable_activatable ( item_listbox );
}

void on_calibration_button_clicked_cb (
        GtkWidget *button,
        WinData *wd
        ) {

    int invisible_win_root_x;
    int invisible_win_root_y;
    int visible_win_root_x;
    int visible_win_root_y;
    int win_width;
    int win_height;

    GdkWindow *win = gtk_widget_get_window ( wd->window );

    gdk_window_get_geometry ( win,
            &invisible_win_root_x, &invisible_win_root_y,
            &win_width, &win_height );

    gdk_window_get_origin ( win,
            &visible_win_root_x, &visible_win_root_y );

    pblue ( "Invisible Window Root Position: %d %d",
            invisible_win_root_x, invisible_win_root_y);

    pblue ( "Visible Window Root Position: %d %d",
            visible_win_root_x, visible_win_root_y);

    /* gtk_window_get_size ( GTK_WINDOW(wd->window), &win_width, &win_height ); */
    pblue ( "Win size: %d %d", win_width, win_height  );

    GList *children = gtk_container_get_children ( GTK_CONTAINER(wd->window) );
    for ( GList *child=children; child!=NULL; child=child->next ) {
        if ( GTK_IS_DRAWING_AREA(child->data) ) {
            printf("Found area\n");
        }
        else if ( GTK_IS_HEADER_BAR(child->data) )
            printf("Is Header Bar\n");
        else if ( GTK_IS_BOX(child->data) )
            printf("Is Box\n");
    }
}


int dataInit(WinData *wd) {

    /*Important: Pay attention to clear the values the global variables*/
    bw.width = 400; bw.height = 100; bw.lines = 0; bw.maxlen = 0;
    mw.width = 400; mw.height = 100; mw.lines = 0; mw.maxlen = 0;
    gw.width = 400; gw.height = 100; gw.lines = 0; gw.maxlen = 0;

    wd->bw = &bw; wd->gw = &gw; wd->mw = &mw;

    wd->gdkwin = NULL;
    wd->width = 400;
    wd->height = 100;
    wd->hadRedirect = 0;
    wd->calibrationButton = NULL;

    wd->quickSearchFlag = FALSE;
    wd->mousePress = FALSE;
    wd->mouseRelease = FALSE;

    moveDone = 0;

    memset ( wd->unselectedButton, '\0', sizeof(wd->unselectedButton) );
    memset ( wd->selectedButton, '\0', sizeof(wd->selectedButton) );
    memset ( wd->needToBeHiddenWidget, '\0', sizeof(wd->needToBeHiddenWidget) );

    focustimes = 1;
    InNewWin = 1;

    /* 窗口打开标志位 changed in captureShortcutEvent.c <变量shmaddr>*/
    shmaddr_keyboard[WINDOW_OPENED_FLAG] = '1';

    wd->gotOfflineTran = 0;
    wd->specific = 0;

    wd->tran_max_len = 0;

    char *shmaddr_setting = NULL;
    shared_memory_for_setting ( &shmaddr_setting );

    wd->shmaddr_setting = shmaddr_setting;
    wd->pinEnable = FALSE;
    wd->openSettingWindowAction = FALSE;
    wd->moveWindowNotify = FALSE;
    wd->doubleClickAction = FALSE;
    wd->previousWidth = 0; 
    wd->previousHeight = 0; 
    wd->showLock = FALSE;
    wd->headerBarWidth  =0;
    wd->headerBarHeight  =0;
    resizeAction = FALSE;
    wd->time = 0;
    wd->beginDrag = FALSE;

    return 0;
}

void makeSegmentationFault (  ) {
    char buf[1];
    if ( sizeof(buf) )  /* prevent anoying warning (set but not used)*/
        buf[9] = '0';
}
gboolean 
on_phonetic_button_clicked_cb ( 
        GtkWidget *button, 
        WinData *wd
        ) {

    mp3play ( button, wd );

    return TRUE;
}

/* From:https://stackoverflow.com/questions/3908565/how-to-make-gtk-window-background-transparent*/
static gboolean expose_draw(GtkWidget *widget, GdkEventExpose *event, WinData *wd ) {

    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));

    gboolean supports_alpha = FALSE;
    GdkScreen *screen = gtk_widget_get_screen(widget);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);

    if (!visual) {
        printf("Your screen does not support alpha channels!\n");
        visual = gdk_screen_get_system_visual(screen);
        supports_alpha = FALSE;
    } else {
        /* printf("Your screen supports alpha channels!\n"); */
        supports_alpha = TRUE;
    }

    gtk_widget_set_visual(widget, visual);
    if (supports_alpha) {
        /* printf("setting transparent window\n"); */
        cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0); 
    } else {
        /* printf("setting opaque window\n"); */
        cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); 
    }

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);

    cairo_destroy(cr);

    return FALSE;
}

gboolean supports_alpha = FALSE;
static void screen_changed(GtkWidget *widget, GdkScreen *old_screen, WinData *wd ) {

    GdkScreen *screen = gtk_widget_get_screen(widget);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);

    if (!visual) {
        /* printf("Your screen does not support alpha channels!\n"); */
        visual = gdk_screen_get_system_visual(screen);
        supports_alpha = FALSE;
    } else {
        /* printf("Your screen supports alpha channels!\n"); */
        supports_alpha = TRUE;
    }

    gtk_widget_set_visual(widget, visual);

}
    gboolean
area_draw_callback (GtkWidget *widget, cairo_t *cr, gint direction )
{

    if ( direction == -1 ) {
        GdkScreen *screen = gtk_widget_get_screen(widget);
        GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
        gtk_widget_set_visual ( widget, visual );
        return TRUE;
    }

    guint width, height;

    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);

    cairo_pattern_t *pattern;

    switch ( direction ) {
        case SOUTH_AREA:
            pattern = cairo_pattern_create_linear ( 0, 0, 0, height ); break;
        case NORTH_AREA:
            pattern = cairo_pattern_create_linear ( 0, height, 0, 0 ); break;
        case WEST_AREA:
            pattern = cairo_pattern_create_linear ( width, 0, 0, 0 ); break;
        case EAST_AREA:
            pattern = cairo_pattern_create_linear ( 0, 0, width, 0 ); break;
        case SOUTH_EAST_AREA:
            pattern = cairo_pattern_create_radial ( 0, 0, 0, 0, 0, SHADOW_BORDER_WIDTH ); break;
        case SOUTH_WEST_AREA:
            pattern = cairo_pattern_create_radial ( width, 0, 0, width, 0, SHADOW_BORDER_WIDTH ); break;
        case NORTH_WEST_AREA:
            pattern = cairo_pattern_create_radial ( width, height, 0, width, height, SHADOW_BORDER_WIDTH ); break;
        case NORTH_EAST_AREA:
            pattern = cairo_pattern_create_radial ( 0, height, 0, 0, height, SHADOW_BORDER_WIDTH ); break;

        default:
            pattern = cairo_pattern_create_linear ( 0, 0, 0, 0 ); break;
    }


    cairo_pattern_add_color_stop_rgba ( pattern, 0, 0, 0, 0, 0.1 );
    cairo_pattern_add_color_stop_rgba ( pattern, 0.3, 0.2, 0.2, 0.2, 0.05 );
    cairo_pattern_add_color_stop_rgba ( pattern, 1, 0.1, 0.1, 0.1, 0 );

    cairo_rectangle ( cr, 0, 0, width, height );
    cairo_set_source ( cr, pattern );
    cairo_pattern_destroy ( pattern );

    cairo_fill (cr);

    return TRUE;
}

/* 代码来源： GTK项目测试用例test/gtktest.c*/
    static gboolean
grippy_button_press (GtkWidget *area, GdkEventButton *event, GdkWindowEdge edge)
{
    if (event->type == GDK_BUTTON_PRESS) 
    {
        if (event->button == GDK_BUTTON_PRIMARY)
            gtk_window_begin_resize_drag (GTK_WINDOW (gtk_widget_get_toplevel (area)), edge,
                    event->button, event->x_root, event->y_root,
                    event->time);
        else if (event->button == GDK_BUTTON_MIDDLE)
            gtk_window_begin_move_drag (GTK_WINDOW (gtk_widget_get_toplevel (area)), 
                    event->button, event->x_root, event->y_root,
                    event->time);
    }

    pyellow ( "Resize Action To TRUE" );
    resizeAction = TRUE;
    return FALSE;
}
void setHeaderBarBackground ( WinData *wd, gchar *color ) {

    GdkRGBA rgba;
    gdk_rgba_parse ( &rgba, color );
    gtk_widget_override_background_color ( wd->headerbar, 0, &rgba );
    gtk_widget_override_background_color ( wd->setting_button, 0, &rgba );
    gtk_widget_override_background_color ( wd->minimizeButton, 0, &rgba );
    gtk_widget_override_background_color ( wd->maximizeButton, 0, &rgba );
    gtk_widget_override_background_color ( wd->destroyButton, 0, &rgba );
}

void shrinkShdowBorder ( WinData *wd ) {

    gtk_widget_set_size_request ( wd->areaWest, 1, -1 );
    gtk_widget_set_size_request ( wd->areaEast, 1, -1 );
    gtk_widget_set_size_request ( wd->areaNorth, -1, 1 );
    gtk_widget_set_size_request ( wd->areaSouth, -1, 1 );

    gtk_widget_set_size_request ( wd->areaNorthWest, 1, 1 );
    gtk_widget_set_size_request ( wd->areaNorthEast, 1, 1 );
    gtk_widget_set_size_request ( wd->areaSouthWest, 1, 1 );
    gtk_widget_set_size_request ( wd->areaSouthEast, 1, 1 );
}

void initShadowBorder ( WinData *wd ) {


    ConfigData *cd = wd->cd;

    if ( cd->shrinkShadowBorder ) {
        shrinkShdowBorder ( wd );
        return;
    }

    gtk_widget_set_app_paintable(wd->window, TRUE);

    if ( ! cd->disableShadowBorder ) {
        g_signal_connect(G_OBJECT(wd->window), "draw", G_CALLBACK(expose_draw), wd );
        g_signal_connect(G_OBJECT(wd->window), "screen-changed", G_CALLBACK(screen_changed), wd);
    }

    screen_changed( wd->window, NULL, NULL );
    g_signal_connect ( wd->areaEast, "draw", G_CALLBACK(area_draw_callback), 
            cd->disableShadowBorder ? (void*)-1 : (void*)EAST_AREA );

    g_signal_connect ( wd->areaWest, "draw", G_CALLBACK(area_draw_callback),
            cd->disableShadowBorder ? (void*)-1 : (void*)WEST_AREA );

    g_signal_connect ( wd->areaNorth, "draw", G_CALLBACK(area_draw_callback), 
            cd->disableShadowBorder ? (void*)-1 : (void*)NORTH_AREA );

    g_signal_connect ( wd->areaSouth, "draw", G_CALLBACK(area_draw_callback), 
            cd->disableShadowBorder ? (void*)-1 : (void*)SOUTH_AREA );

    g_signal_connect ( wd->areaNorthEast, "draw", G_CALLBACK(area_draw_callback), 
            cd->disableShadowBorder ? (void*)-1 : (void*)NORTH_EAST_AREA );

    g_signal_connect ( wd->areaNorthWest, "draw", G_CALLBACK(area_draw_callback), 
            cd->disableShadowBorder ? (void*)-1 : (void*)NORTH_WEST_AREA );

    g_signal_connect ( wd->areaSouthEast, "draw", G_CALLBACK(area_draw_callback), 
            cd->disableShadowBorder ? (void*)-1 : (void*)SOUTH_EAST_AREA );

    g_signal_connect ( wd->areaSouthWest, "draw", G_CALLBACK(area_draw_callback), 
            cd->disableShadowBorder ? (void*)-1 : (void*)SOUTH_WEST_AREA );

    int mask = GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK ;
    gtk_widget_add_events (wd->areaEast, mask);
    gtk_widget_add_events (wd->areaWest, mask);
    gtk_widget_add_events (wd->areaNorth, mask);
    gtk_widget_add_events (wd->areaSouth, mask);
    gtk_widget_add_events (wd->areaNorthEast, mask);
    gtk_widget_add_events (wd->areaNorthWest, mask);
    gtk_widget_add_events (wd->areaSouthEast, mask);
    gtk_widget_add_events (wd->areaSouthWest, mask);

    gtk_widget_set_events (wd->areaEast, gtk_widget_get_events (wd->areaEast)
            | GDK_BUTTON_PRESS_MASK
            | GDK_POINTER_MOTION_MASK
            | GDK_BUTTON_RELEASE_MASK
            |GDK_LEAVE_NOTIFY_MASK);

    g_signal_connect (wd->areaNorthWest, "button-press-event",
            G_CALLBACK (grippy_button_press), GINT_TO_POINTER (GDK_WINDOW_EDGE_NORTH_WEST));
    g_signal_connect (wd->areaNorth, "button-press-event", 
            G_CALLBACK (grippy_button_press), GINT_TO_POINTER (GDK_WINDOW_EDGE_NORTH));
    g_signal_connect (wd->areaNorthEast, "button-press-event", 
            G_CALLBACK (grippy_button_press), GINT_TO_POINTER (GDK_WINDOW_EDGE_NORTH_EAST));
    g_signal_connect (wd->areaWest, "button-press-event", 
            G_CALLBACK (grippy_button_press), GINT_TO_POINTER (GDK_WINDOW_EDGE_WEST));
    g_signal_connect (wd->areaEast, "button-press-event", 
            G_CALLBACK (grippy_button_press), GINT_TO_POINTER (GDK_WINDOW_EDGE_EAST));
    g_signal_connect (wd->areaSouthWest, "button-press-event", 
            G_CALLBACK (grippy_button_press), GINT_TO_POINTER (GDK_WINDOW_EDGE_SOUTH_WEST));
    g_signal_connect (wd->areaSouth, "button-press-event", 
            G_CALLBACK (grippy_button_press), GINT_TO_POINTER (GDK_WINDOW_EDGE_SOUTH));
    g_signal_connect (wd->areaSouthEast, "button-press-event", 
            G_CALLBACK (grippy_button_press), GINT_TO_POINTER (GDK_WINDOW_EDGE_SOUTH_EAST));


    g_signal_connect ( wd->areaEast, "motion-notify-event", G_CALLBACK(on_motion_notify_event), wd );
    g_signal_connect ( wd->areaWest, "motion-notify-event", G_CALLBACK(on_motion_notify_event), wd );
    g_signal_connect ( wd->areaSouth, "motion-notify-event", G_CALLBACK(on_motion_notify_event), wd );
    g_signal_connect ( wd->areaNorth, "motion-notify-event", G_CALLBACK(on_motion_notify_event), wd );
    g_signal_connect ( wd->areaNorthEast, "motion-notify-event", G_CALLBACK(on_motion_notify_event), wd );
    g_signal_connect ( wd->areaSouthEast, "motion-notify-event", G_CALLBACK(on_motion_notify_event), wd );
    g_signal_connect ( wd->areaNorthWest, "motion-notify-event", G_CALLBACK(on_motion_notify_event), wd );
    g_signal_connect ( wd->areaSouthWest, "motion-notify-event", G_CALLBACK(on_motion_notify_event), wd );
}

void initObjectFromFile ( WinData *wd ) {

    GtkBuilder *builder = gtk_builder_new_from_file(expanduser(UI_SRC));
    GtkWidget *window = (GtkWidget*)gtk_builder_get_object ( builder, "root" );
    GtkWidget *audio_button_en =  (GtkWidget*)gtk_builder_get_object ( builder, "audio_button_en" );
    GtkWidget *audio_button_am =  (GtkWidget*)gtk_builder_get_object ( builder, "audio_button_am" );
    GtkWidget *phonetic_en =  (GtkWidget*)gtk_builder_get_object ( builder, "phonetic_en" );
    GtkWidget *phonetic_am =  (GtkWidget*)gtk_builder_get_object ( builder, "phonetic_am" );
    GtkWidget *src_label =  (GtkWidget*)gtk_builder_get_object ( builder, "source_label" );
    GtkWidget *box =  (GtkWidget*)gtk_builder_get_object ( builder, "box" );
    GtkWidget *headerbar =  (GtkWidget*)gtk_builder_get_object ( builder, "header_bar" );
    GtkWidget *headerBox = (GtkWidget*)gtk_builder_get_object ( builder, "header_box" ); 
    GtkWidget *phon_listbox = (GtkWidget*)gtk_builder_get_object ( builder, "phonetic_listbox" );
    GtkWidget *ctrl_listbox = (GtkWidget*)gtk_builder_get_object ( builder, "control_listbox" );
    GtkWidget *ctrl_grid_src = (GtkWidget*)gtk_builder_get_object ( builder, "control_grid" );
    GtkWidget *content_listbox = (GtkWidget*)gtk_builder_get_object ( builder, "content_listbox" );
    GtkWidget *content_box = (GtkWidget*)gtk_builder_get_object ( builder, "content_box" );
    GtkWidget *calibrationButton = (GtkWidget*)gtk_builder_get_object ( builder, "calibration_button" );
    GtkWidget *exitbutton = (GtkWidget*)gtk_builder_get_object ( builder, "exit_button" );
    GtkWidget *pinbutton = (GtkWidget*)gtk_builder_get_object ( builder, "pin_button" );
    GtkWidget *bingbutton = (GtkWidget*)gtk_builder_get_object ( builder, "bing_button" );
    GtkWidget *offlinebutton = (GtkWidget*)gtk_builder_get_object ( builder, "offline_button" );
    GtkWidget *googlebutton = (GtkWidget*)gtk_builder_get_object ( builder, "google_button" );
    GtkWidget *src_listbox = (GtkWidget*)gtk_builder_get_object ( builder, "src_listbox" );
    GtkWidget *areaWest = (GtkWidget*)gtk_builder_get_object ( builder, "area_west" );
    GtkWidget *areaEast = (GtkWidget*)gtk_builder_get_object ( builder, "area_east" );
    GtkWidget *areaNorth = (GtkWidget*)gtk_builder_get_object ( builder, "area_north" );
    GtkWidget *areaSouth = (GtkWidget*)gtk_builder_get_object ( builder, "area_south" );
    GtkWidget *areaSouthEast = (GtkWidget*)gtk_builder_get_object ( builder, "area_south_east" );
    GtkWidget *areaSouthWest = (GtkWidget*)gtk_builder_get_object ( builder, "area_south_west" );
    GtkWidget *areaNorthEast = (GtkWidget*)gtk_builder_get_object ( builder, "area_north_east" );
    GtkWidget *areaNorthWest = (GtkWidget*)gtk_builder_get_object ( builder, "area_north_west" );
    GtkWidget *bottomBox = (GtkWidget*)gtk_builder_get_object ( builder, "bottom_box" ); 
    GtkWidget *topBox = (GtkWidget*)gtk_builder_get_object ( builder, "top_box" ); 
    GtkWidget *rightBox = (GtkWidget*)gtk_builder_get_object ( builder, "right_box" ); 
    GtkWidget *leftBox = (GtkWidget*)gtk_builder_get_object ( builder, "left_box" ); 
    GtkWidget *mainBox = (GtkWidget*)gtk_builder_get_object ( builder, "main_box" ); 
    GtkWidget *minimizeButton = (GtkWidget*)gtk_builder_get_object ( builder, "minimize_button" ); 
    GtkWidget *maximizeButton = (GtkWidget*)gtk_builder_get_object ( builder, "maximize_button" ); 
    GtkWidget *destroyButton = (GtkWidget*)gtk_builder_get_object ( builder, "destroy_button" ); 

    GtkWidget *setting_button = NULL;
    GtkWidget *setting_button_bottom = NULL;
    setting_button_bottom = (GtkWidget*)gtk_builder_get_object ( builder, "setting_button_bottom" );
    setting_button = (GtkWidget*)gtk_builder_get_object ( builder, "setting_button" );


    wd->window = window;
    wd->unselectedGoogle = wd->unselectedButton[0] = googlebutton;
    wd->unselectedOffline = wd->unselectedButton[1] = offlinebutton;
    wd->unselectedBing = wd->unselectedButton[2] = bingbutton;
    wd->unselectedPin = wd->unselectedButton[3] = pinbutton;
    wd->unselectedPin = pinbutton;
    wd->headerbar = headerbar;
    wd->ctrl_grid = ctrl_grid_src;
    wd->setting_button = setting_button;
    wd->setting_button_bottom = setting_button_bottom;
    wd->content_listbox = content_listbox;
    wd->googleButton = googlebutton;
    wd->baiduButton = bingbutton;
    wd->mysqlButton = offlinebutton;
    wd->src_label = src_label;
    wd->audio_button_am = audio_button_am;
    wd->audio_button_en = audio_button_en;
    wd->phonetic_en = phonetic_en;
    wd->phonetic_am = phonetic_am;
    wd->phon_listbox = phon_listbox;
    wd->box = box;
    wd->ctrl_listbox = ctrl_listbox;
    wd->content_box = content_box;
    wd->src_listbox = src_listbox;
    wd->exitButton = exitbutton;
    wd->calibrationButton = calibrationButton;
    wd->areaEast = areaEast ;
    wd->areaWest = areaWest ;
    wd->areaNorth = areaNorth ;
    wd->areaSouth = areaSouth ;
    wd->areaSouthEast = areaSouthEast ;
    wd->areaSouthWest = areaSouthWest ;
    wd->areaNorthEast = areaNorthEast ;
    wd->areaNorthWest = areaNorthWest ;
    wd->bottomBox = bottomBox;
    wd->topBox = topBox;
    wd->leftBox = leftBox;
    wd->rightBox = rightBox;
    wd->headerBox = headerBox;
    wd->minimizeButton = minimizeButton;
    wd->maximizeButton = maximizeButton;
    wd->destroyButton = destroyButton;
    wd->mainBox = mainBox;

    addUnderline (src_label, "#581880", PANGO_UNDERLINE_LOW);

    setWidgetProperties(phonetic_en, 1.1, "#00aaff", NOT_BOLD, NOT_TRANSPARENT);
    setWidgetProperties(phonetic_am, 1.1, "#00aaff", NOT_BOLD, NOT_TRANSPARENT);

    setHeaderBarBackground  ( wd, "#494949" );


    gtk_window_set_default_size(GTK_WINDOW(wd->window), 400, 100);
    gtk_window_set_keep_above ( GTK_WINDOW(wd->window), TRUE );
    gtk_window_set_title(GTK_WINDOW(wd->window), "");

    initShadowBorder ( wd );

    g_signal_connect(G_OBJECT(wd->window), "destroy", \
            G_CALLBACK(destroyNormalWin), wd);

    g_signal_connect(G_OBJECT(wd->window), "key-press-event", \
            G_CALLBACK(on_key_press_cb), wd);

    g_signal_connect ( calibrationButton, "clicked",
            G_CALLBACK(on_calibration_button_clicked_cb), wd );

    g_signal_connect ( exitbutton, "clicked",
            G_CALLBACK(destroyNormalWin), wd );
    g_signal_connect ( window, "size-allocate",
            G_CALLBACK(on_win_size_allocate_cb), wd );

    g_signal_connect ( googlebutton, "clicked",
            G_CALLBACK(on_tran_button_clicked_cb), wd);
    g_signal_connect ( bingbutton, "clicked",
            G_CALLBACK(on_tran_button_clicked_cb), wd);
    g_signal_connect ( offlinebutton, "clicked",
            G_CALLBACK(on_tran_button_clicked_cb), wd);

    g_signal_connect ( pinbutton, "clicked",
            G_CALLBACK(on_pin_button_clicked_cb), wd);

    g_signal_connect ( audio_button_am, "clicked", 
            G_CALLBACK(on_phonetic_button_clicked_cb), wd );

    g_signal_connect ( audio_button_en, "clicked", 
            G_CALLBACK(on_phonetic_button_clicked_cb), wd );


    g_signal_connect ( maximizeButton, "clicked", G_CALLBACK(on_maximize_button_clicked_cb), wd );
    g_signal_connect ( minimizeButton, "clicked", G_CALLBACK(on_minimize_button_clicked_cb), wd );
    g_signal_connect ( destroyButton, "clicked", G_CALLBACK(destroyNormalWin), wd );

    gtk_window_set_decorated ( GTK_WINDOW(window), FALSE );
}

void on_setting_button_clicked_cb (
        GtkWidget *button,
        WinData *wd
        ) {

    /* 设置窗口打开标志，在设置窗口的逻辑里会执行这个操作，但是由于
     * 时间差，会导致本文件中检测设置窗口是否已经关闭时产生误判，所以
     * 这里先行标记*/
    wd->shmaddr_setting[1] = '1';

    wd->openSettingWindowAction = TRUE;
    wd->shmaddr_setting[0] = '1';
}

void initSettingButton ( WinData *wd ) {

    ConfigData *cd = wd->cd;
    GtkWidget *button = NULL;

    if ( ! cd->hideHeaderBar ) {
        button = wd->setting_button;
        addToHiddenArray ( wd->setting_button_bottom, wd );
    }
    else
        button = wd->setting_button_bottom;

    g_signal_connect ( button, "clicked", 
            G_CALLBACK(on_setting_button_clicked_cb), wd );
}

void setBackground( WinData *wd ) {

    GdkRGBA rgba;
    gdk_rgba_parse ( &rgba, "#fafafa" );
    wd->rgba = rgba;
    gtk_widget_override_background_color ( wd->box, 0, &rgba );
    gtk_widget_override_background_color ( wd->window, 0, &rgba );
    gtk_widget_override_background_color ( wd->src_label, 0, &rgba );
    gtk_widget_override_background_color ( wd->phonetic_en, 0, &rgba );
    gtk_widget_override_background_color ( wd->phonetic_am, 0, &rgba );
    gtk_widget_override_background_color ( wd->src_listbox, 0, &rgba );
    gtk_widget_override_background_color ( wd->item_label, 0, &rgba );
    gtk_widget_override_background_color ( wd->phon_listbox, 0, &rgba );
    gtk_widget_override_background_color ( wd->ctrl_listbox, 0, &rgba );
    gtk_widget_override_background_color ( wd->content_listbox, 0, &rgba );
    gtk_widget_override_background_color ( wd->selectedPin, 0, &rgba );
    gtk_widget_override_background_color ( wd->unselectedPin, 0, &rgba );
    gtk_widget_override_background_color ( wd->unselectedBing, 0, &rgba );
    gtk_widget_override_background_color ( wd->selectedBing, 0, &rgba );
    gtk_widget_override_background_color ( wd->selectedGoogle, 0, &rgba );
    gtk_widget_override_background_color ( wd->unselectedGoogle, 0, &rgba );
    gtk_widget_override_background_color ( wd->unselectedOffline, 0, &rgba );
    gtk_widget_override_background_color ( wd->selectedOffline, 0, &rgba );
    gtk_widget_override_background_color ( wd->exitButton, 0, &rgba );
    gtk_widget_override_background_color ( wd->calibrationButton, 0, &rgba );
    gtk_widget_override_background_color ( wd->audio_button_am, 0, &rgba );
    gtk_widget_override_background_color ( wd->audio_button_en, 0, &rgba );
}

int areaToDirection ( GtkWidget *widget, WinData *wd  ) {

    int direction = -1;

    if ( widget == wd->areaNorth ) direction = NORTH_AREA;
    if ( widget == wd->areaNorthEast ) direction = NORTH_EAST_AREA;
    if ( widget == wd->areaNorthEast ) direction = NORTH_EAST_AREA;
    if ( widget == wd->areaEast ) direction = EAST_AREA;
    if ( widget == wd->areaSouthEast ) direction = SOUTH_EAST_AREA;
    if ( widget == wd->areaSouth) direction = SOUTH_AREA;
    if ( widget == wd->areaSouthWest) direction = SOUTH_WEST_AREA;
    if ( widget == wd->areaWest) direction = WEST_AREA;
    if ( widget == wd->areaNorthWest) direction = NORTH_WEST_AREA;

    return direction;
}

gboolean 
on_motion_notify_event (
        GtkWidget *widget,
        GdkEventButton *event,
        WinData *wd) {

    static char cursors[][10] = {
        "n-resize","ne-resize", "e-resize","se-resize",
        "s-resize","sw-resize","w-resize","nw-resize"
    };

    int i = areaToDirection ( widget, wd );

    if ( i == -1 ) return FALSE;

    GdkCursor *cursor = gdk_cursor_new_from_name (gtk_widget_get_display (widget), cursors[i]);
    gdk_window_set_cursor (gtk_widget_get_window (widget), cursor);
    g_object_unref (cursor);

    resizeAction = TRUE;

    return TRUE;
}

gboolean on_cnfigure_event_cb (
        GtkWidget *widget,
        GdkEvent *event,
        WinData *wd) {

    /* printf("Configure event\n"); */

    return FALSE;
}

/*新建翻译结果窗口, 本文件入口函数*/
void *newNormalWindow ( void *data ) {

    /* makeSegmentationFault(); */

    ConfigData *cd = data;
    static WinData wd;
    wd.cd = cd;

    dataInit(&wd);

    if ( waitForContinue( &wd ) ) 
    {
        InNewWin = 0;
        return (void*)0;
    }

    gtk_init(NULL, NULL);

    initObjectFromFile(&wd);
    initSettingButton(&wd);
    /* setBackground( &wd ); */

    /* quickSearch快捷键标志位, 在shortcutListener中置位*/
    if ( shmaddr_keyboard[QUICK_SEARCH_NOTIFY] == '1') {

        shmaddr_keyboard[QUICK_SEARCH_NOTIFY] = '0';
        gtk_window_set_position(GTK_WINDOW(wd.window), GTK_WIN_POS_CENTER);
        wd.quickSearchFlag = TRUE;
    }
    else 
    {
        gtk_window_set_position(GTK_WINDOW(wd.window), GTK_WIN_POS_MOUSE);
    }

    /* printDebugInfo(); */

    /*初始化百度以及离线翻译结果存储空间*/
    initMemoryBaidu();
    initMemoryMysql();
    initMemoryTmp();
    initMemoryGoogle();

    g_signal_connect (wd.window, "button-press-event", 
            G_CALLBACK(on_button_press_cb), &wd);
    g_signal_connect ( wd.window, "configure-event", 
            G_CALLBACK(on_cnfigure_event_cb), &wd );

    timeout_id = g_timeout_add(10, focus_request, &wd);
    if ( ! wd.quickSearchFlag ) {
        pgreen ( "启动窗口位置超时检测函数" );
        movewindow_timeout_id  = g_timeout_add ( 50, check_pointer_and_window_position, &wd );
    }

    loadSelectedButton ( &wd );

    if ( cd->hideHeaderBar )
        addToHiddenArray ( wd.headerbar, &wd );

    selectDisplay ( &wd );

    gtk_main();

    pthread_exit(NULL);
}

int destroyNormalWin(GtkWidget *unKnowWidget, WinData *wd) {

    pbblue ( "Destroy window" );

    clearMemory();

    /* 窗口关闭标志位*/
    shmaddr_keyboard[WINDOW_OPENED_FLAG] = '0';
    shmaddr_keyboard[CTRL_C_PRESSED_FLAG] = '0';

    /*清除相关标记*/
    shmaddr_baidu[0] = CLEAR;
    shmaddr_google[0] = CLEAR;

    action = 0;

    pbcyan ( "InNewWin 置零" );

    /* 已退出翻译结果窗口，重置标志变量*/
    InNewWin = 0;


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

    pgreen("准备接收共享内存数据... ( WaitForContinue )");

    int time = 0;

    /*等待任意一方python端的翻译数据全部写入共享内存*/
    while( (
                shmaddr_google[0] != FINFLAG && 
                shmaddr_baidu [0] != FINFLAG &&
                shmaddr_mysql [0] != FINFLAG )) {

        /*长时间未检测到共享内存里的数据进行双击取消本次窗口显示*/
        if ( action == DOUBLE_CLICK ) {

            pred("双击退出. ( WaitForContinue )");
            action = 0;
            return 1;
        }
        if ( shmaddr_google[0] == ERRCHAR) {
            pred("翻译过程出现错误 ( WaitForContinue )");
            break;
        }

        usleep(400000);

        /* 超时时间1.6S ( 2 * 400000ms )*/
        if ( ++time >= 2 ) {
            pred("超时退出");
            shmaddr_google[0] = ERRCHAR;
            shmaddr_baidu[0] = ERRCHAR;
            break;
        }
    }

    if ( text == NULL )
        text = calloc(TEXTSIZE, 1);

    action = 0;
    wd->gotOfflineTran = ( shmaddr_mysql [0] == FINFLAG ) ? 1 : 0;
    wd->gotGoogleTran  = ( shmaddr_google[0] == FINFLAG ) ? 1 : 0;
    wd->gotBaiduTran   = ( shmaddr_baidu [0] == FINFLAG ) ? 1 : 0;

    return 0;
}
/* 重新从共享内存获取百度翻译结果并设置窗口大小*/
int reGetBaiduTrans (gpointer *data, int who ) {

    int index[INDEX_SIZE] = { 0 };
    int ret = 0;
    ret = getIndex(index, GET_SHMADDR(who) );
    if ( ret ) return ret;

    int len = 30;
    separateDataForBaidu(index, len, TYPE(who) );

    return 0;
}

int refreshResult(GtkWidget *button, gpointer *data, int who ) {

    int ret = 0;

    if ( who == GOOGLE ) 
    {
        int index[2] = { 0 };
        ret = getIndex(index, shmaddr_google);
        if ( ret ) return ret;

        /* 找到分割符，数据分离提取才有意义*/
        if ( index[0] != 0 )
            separateGoogleData ( index, 28 );
        else
            pred("未找到分隔符(refreshResult)");
    }
    else if ( who == BAIDU || who == MYSQL)
    {
        ret = reGetBaiduTrans ( data, who );
        if ( ret ) return ret;
    }

    return 0;
}

static inline int nextWindow ( int who )  {
    return who < 3 ? who + 1 : 1;
}

/* 切换各个翻译结果的显示*/
int changeDisplay(GtkWidget *button, WinData *wd) {

    wd->who = nextWindow((wd->who));
    button = GET_BUTTON ( wd, wd->who );

    on_tran_button_clicked_cb ( button, wd );

    return 0;
}

/* 删除字符串末尾遇到的第一个回车符*/
char *removeTrailCR ( char *str ) {

    if ( !str ) return NULL;

    char *p = str;

    /* 使指针指向字符串末尾*/
    while ( *p++ );
    p = p - 2;

    /* 回退p，使其指向遇到的第一个回车符,如果找到则替换成'\0'*/
    while ( p != str && *p != '\n' ) p--;
    if ( *p == '\n' ) *p = '\0';

    return str;
}

void displayGoogleTrans(GtkWidget *button, gpointer *data) {

    pyellow("显示谷歌翻译结果:\n");

    WinData *wd = WINDATA(data);
    char **result = google_result;
    char *p1 = NULL;
    char *p2 = NULL;

    wd->who = GOOGLE;
    wd->specific = 1;

    refreshResult ( button, data, GOOGLE );

    clearContentListBox ( wd->content_listbox );

    if ( strlen( text )  < 30 )
        gtk_label_set_text ( GTK_LABEL(wd->src_label), removeTrailCR(text) );

    setWidgetProperties ( wd->src_label, 1.3, "#000000", BOLD_TYPE, NOT_TRANSPARENT );

    if ( result[0][0] ) 
        insertTextContentBox ( "翻译", result[0], wd, "#216459");


    if ( result[1][0] ) {
        p1 = "英译英";
        p2 = result[1];
        insertTextContentBox ( p1, p2, wd , "#606415");
    }

    if ( result[2][0] ) {
        getPosTran ( result[2], &p1, &p2, ':' );
        insertTextContentBox ( p1, p2, wd, "#242783" );
    }
}

void displayTrans ( WinData *wd, char ***result  ) {

    char *p1 = NULL;
    char *p2 = NULL;

    void getPhonetic ( char *src, char **phonetic_en, char **phonetic_am );
    getPhonetic ( result[1][0], &p1, &p2 );

    if ( strlen(result[0][0]) < 30 )
        gtk_label_set_text ( GTK_LABEL(wd->src_label), result[0][0] );

    setWidgetProperties ( wd->src_label, 1.3, "#000000", BOLD_TYPE, NOT_TRANSPARENT );

    gtk_label_set_text ( GTK_LABEL(wd->phonetic_en), p1 );
    gtk_label_set_text ( GTK_LABEL(wd->phonetic_am ), p2 );

    void clearContentListBox ( GtkWidget *listbox  );
    clearContentListBox ( wd->content_listbox );

    for ( int i=0; i<ZH_EN_TRAN_SIZE && result[2][i][0]; i++ ) {

        getPosTran ( result[2][i], &p1, &p2, '.' );
        insertTextContentBox ( p1, p2, wd, "#216459" );
    }


    for ( int i=0; i<ZH_EN_TRAN_SIZE && result[3][i][0]; i++ ) {
        insertTextContentBox ( "英译", result[3][i], wd , "#606415");
    }

    if ( result[4][0][0] )
        insertTextContentBox ( "其它形式", result[4][0], wd , "#242783");

    gtk_widget_set_size_request ( wd->window, wd->tran_max_len*10, -1 );
    show_all_visible_widget ( wd );

    setWidgetProperties(wd->phonetic_en, 1.1, "#00aaff", NOT_BOLD, NOT_TRANSPARENT);
    setWidgetProperties(wd->phonetic_am, 1.1, "#00aaff", NOT_BOLD, NOT_TRANSPARENT);
}

void displayOfflineTrans ( GtkWidget *button, gpointer *data ) {

    pmag("显示离线翻译:\n");

    WinData *wd = WINDATA(data);
    char ***result = mysql_result;

    wd->who = MYSQL;
    wd->specific = 1;

    refreshResult ( button, data, MYSQL );

    displayTrans ( wd, result );
}

void getPhonetic ( char *src, char **phonetic_en, char **phonetic_am ) {

    static char empty_en[] = "  英: 无";
    static char empty_am[] = "  美: 无";
    static char buf[512] = { '\0' };

    *phonetic_en = empty_en;
    *phonetic_am = empty_am;
    if ( !src[0] ) return;

    strcpy ( buf, src );

    char *p = strstr ( buf, "英" );

    if ( !p ) return;

    /* *( p-2 ) = '\n'; */
    *( p-1 ) = '\0'; 
    *phonetic_en = buf;
    *phonetic_am = p;

    while ( *p && *p != '\n') p++;
    if ( *p == '\n' ) *p = '\0';
}

void clearContentListBox ( GtkWidget *listbox  ) {

    inline void destroy_widget ( GtkWidget *widget, gpointer data ) {
        gtk_widget_destroy ( widget );
    }

    gtk_container_forall ( 
            GTK_CONTAINER(listbox),
            destroy_widget,
            NULL
            );
}

/* Get part of speech and translation*/
void getPosTran ( char *src, char **pos, char **tran, char split ) {

    static char default_pos[] = "NaN.";
    static char default_tran[] = "NaN.";
    static char posbuf[32] = "\0";
    char *p = NULL;

    *pos = default_pos;
    *tran = default_tran;

    if ( !src[0] ) return;

    p = strchr ( src, split );
    if ( !p || p-src>15) return;

    strncpy ( posbuf, src, p-src+1 );
    posbuf[p-src] = '\0';

    *pos = posbuf;
    *tran = p+1;
}

void displayBaiduTrans(GtkWidget *button,  gpointer *data ) {

    WinData *wd = WINDATA(data);
    char ***result = baidu_result;

    wd->who = BAIDU;
    wd->specific = 1;

    refreshResult ( button, data, BAIDU );

    displayTrans ( wd, result );
}

GtkWidget *addUnderline ( 
        GtkWidget *widget,
        const char *rgb,
        guint type
        ) {

    PangoColor color;
    pango_color_parse ( &color, rgb );
    guint red = color.red;
    guint green = color.green;
    guint blue = color.blue;

    PangoAttrList *pangoAttrList = NULL;

    if ( GTK_IS_LABEL(widget) ) {
        pangoAttrList = gtk_label_get_attributes(GTK_LABEL(widget));
        if ( ! pangoAttrList ) pangoAttrList = pango_attr_list_new();
    }
    else if ( GTK_IS_ENTRY(widget) ) {

        pangoAttrList = gtk_entry_get_attributes(GTK_ENTRY(widget));
        if ( ! pangoAttrList ) pangoAttrList = pango_attr_list_new();
    }

    PangoAttribute *pangoAttribute;
    pangoAttribute = pango_attr_underline_new(PANGO_UNDERLINE_LOW);
    pango_attr_list_insert ( pangoAttrList, pangoAttribute ); 

    pangoAttribute = pango_attr_underline_color_new(red, green, blue); /* Color Cyan*/
    pango_attr_list_insert ( pangoAttrList, pangoAttribute ); 


    if ( GTK_IS_LABEL(widget) )
        gtk_label_set_attributes ( GTK_LABEL(widget), pangoAttrList ) ;

    else if ( GTK_IS_ENTRY(widget) ) 
        gtk_entry_set_attributes ( GTK_ENTRY(widget), pangoAttrList ) ;

    return widget;
}

GtkWidget *setWidgetProperties ( 

        GtkWidget *widget, 
        double fontSizeScale,
        const char *rgb,
        int bold,
        int alpha
        ) {

    if ( ! widget  ) return NULL;

    GdkRGBA color;
    gdk_rgba_parse ( &color, rgb );

    PangoAttrList *pangoAttrList = NULL;

    if ( GTK_IS_LABEL(widget) ) {
        pangoAttrList = gtk_label_get_attributes(GTK_LABEL(widget));
        if ( ! pangoAttrList ) pangoAttrList = pango_attr_list_new();
    }
    else if ( GTK_IS_ENTRY(widget) ) {

        pangoAttrList = gtk_entry_get_attributes(GTK_ENTRY(widget));
        if ( ! pangoAttrList ) pangoAttrList = pango_attr_list_new();
    }
    PangoAttribute *pangoAttribute ;
    if ( bold ) {
        pangoAttribute = pango_attr_weight_new ( PANGO_WEIGHT_BOLD );
        pango_attr_list_insert ( pangoAttrList, pangoAttribute ); 
    }

    pangoAttribute = pango_attr_scale_new( fontSizeScale ); 
    pango_attr_list_insert ( pangoAttrList, pangoAttribute ); 

    gtk_widget_override_color( widget, 0,  &color);

    if ( GTK_IS_LABEL(widget) )
        gtk_label_set_attributes ( GTK_LABEL(widget), pangoAttrList ) ;

    else if ( GTK_IS_ENTRY(widget) ) 
        gtk_entry_set_attributes ( GTK_ENTRY(widget), pangoAttrList ) ;

    return widget;

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
