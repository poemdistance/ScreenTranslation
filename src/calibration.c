#include "common.h"
#include "windowData.h"
#include "newWindow.h"
#include "calibration.h"
#include "audio.h"
#include "fitting.h" /* notExist()*/
#include "expanduser.h"

void writeAudioButtonInfo ( int l, int x, int y , int disable) {

    if ( disable )
        return;

    char bash[140] = "bash ";
    char num[20] = { '\0' };
    char *path = expanduser ( "/home/$USER/.stran/buttonPositionDataCtl.sh" );
    char *file = expanduser("/home/$USER/.stran/"AUDIO_BASE_NAME".data");

    if ( notExist ( path ) ) {
        pbred ( "%s not exist ( writeAudioButtonInfo )", path );
        return;
    }

    if ( notExist ( file ) ) {
        pbred ( "%s not exist, creating it...", file );
        fclose ( fopen ( file, "w" ) ); /* 这种写法不好，open可能返回空导致段错误,但是简洁...*/
    }

    /* 注意不要忽略3个 %d 前后的空格, 否则bash命令组装错误无法正确运行*/
    sprintf(num, " %d %d %d ", l, x, y );
    system ( strcat ( strcat ( strcat ( bash, path ), num ), file ) );
}

void writeWinSizeInfo ( int linelen, int width, int linenum, int height, int disable ) {

    if ( disable )
        return;

    char bash[140] = "bash ";
    char num[20] = { '\0' };
    char *path = expanduser("/home/$USER/.stran/winSizeDataCtl.sh");
    char *file1 = expanduser("/home/$USER/.stran/"WIN_SIZE_BASE_NAME"_part1.data");
    char *file2 = expanduser("/home/$USER/.stran/"WIN_SIZE_BASE_NAME"_part2.data");

    if ( notExist ( path ) ) {
        pbred ( "%s not exist ( writeAudioButtonInfo )", path );
        return;
    }

    if ( notExist ( file1 ) ) {
        pbred ( "%s not exist, creating it...", file1 );
        fclose ( fopen ( file1, "w" ) );
    }
    if ( notExist ( file2 ) ) {
        pbred ( "%s not exist, creating it...", file2 );
        fclose ( fopen ( file2, "w" ) );
    }

    sprintf(num, " %d %d ", linelen, width );
    system ( strcat ( strcat ( strcat ( bash, path ), num ), file1 ) );

    memset ( num, '\0', sizeof(num) );
    memset ( bash, '\0', sizeof(bash) );
    sprintf(num, " %d %d ", linenum, height );
    system ( strcat ( strcat ( strcat ( bash, path ), num ), file2 ) );
}

void recordWinInfo ( GtkWidget *button, gpointer *data ) {

    WinData *win = WINDATA(data);
    gint width = 0, height = 0;
    gtk_window_get_size ( GTK_WINDOW(win->window), &width, &height );

    pbcyan("行数|窗高-单行最长字数|窗宽 --音标长|-x|-y");
    pbyellow("%d %d   %d %d  %d %d %d\n",\
            GET_DISPLAY_LINES_NUM ( win, win->who ),height,\
            GET_DISPLAY_MAX_LEN ( win, win->who ), width,\
            countCharNums ( Phonetic ( TYPE ( WINDATA(data)->who ) ) ),\
            WINDATA(data)->ox, WINDATA(data)->oy\
            );

    writeAudioButtonInfo (
            countCharNums ( Phonetic ( TYPE ( WINDATA(data)->who ) ) ),\
            WINDATA(data)->ox, WINDATA(data)->oy, FITTING_STATUS);

    writeWinSizeInfo ( \
            GET_DISPLAY_MAX_LEN ( win, win->who ), width,\
            GET_DISPLAY_LINES_NUM ( win, win->who ), height, FITTING_STATUS );
}

void insertCalibrationButton( WinData *win ) {

    GtkWidget* button =  newCalibrationButton ( win );

    gtk_layout_put ( GTK_LAYOUT(win->layout), button,\
            win->width-RIGHT_BORDER_OFFSET*4, win->height-BOTTOM_OFFSET );

    g_signal_connect ( button, "clicked",\
            G_CALLBACK(recordWinInfo), (void*)win );

    gtk_widget_show(button);
}

gboolean deal_motion_notify_event ( GtkWidget *widget, GdkEventMotion *event, gpointer *data)
{
    int wx, wy; /* The absolutely position of the left up corner of window*/
    int rx, ry; /* The position of the pointer relative to the left up corner of window*/

    gdk_window_get_position ( gtk_widget_get_window(WINDATA(data)->window), &wx, &wy );

    rx = (int)event->x_root - wx;
    ry = (int)event->y_root - wy;

    gtk_window_get_size (  (GtkWindow*)WINDATA(data)->window ,\
            &WINDATA(data)->width, &WINDATA(data)->height );

    /* 在Button上方按下按键后使能拖拽标志位,直到释放按钮清空该标志*/
    if ( WINDATA(data)->press &&  WINDATA(data)->enter )
        WINDATA(data)->drag = 1;

    /* 防止超出屏幕*/
    if ( rx > WINDATA(data)->width )
        rx = WINDATA(data)->width ;
    else if ( rx <= 0 )
        rx = 0;

    /* 防止超出屏幕*/
    if ( ry > WINDATA(data)->height )
        ry = WINDATA(data)->height ;
    else if ( ry <= 0 )
        ry = 0;

    if ( WINDATA(data)->drag ) {

        WINDATA(data)->ox = rx-WINDATA(data)->cx;
        WINDATA(data)->oy = ry-WINDATA(data)->cy;

        gtk_layout_move ( GTK_LAYOUT(WINDATA(data)->layout), WINDATA(data)->audio,\
                rx-WINDATA(data)->cx, ry-WINDATA(data)->cy );
    }

    return FALSE;
}

gboolean enter_button (GtkWidget *widget, GdkEventKey *event, gpointer *data) {

    //printf("Enter button\n");
    WINDATA(data)->enter = 1;
    return FALSE;
}

gboolean leave_button (GtkWidget *widget, GdkEventKey *event, gpointer *data) {

    //printf("Leave button\n");
    WINDATA(data)->enter = 0;
    return FALSE;
}

gboolean press_button (GtkWidget *widget, GdkEventKey *event, gpointer *data) {

    //printf("Press Button\n");
    WINDATA(data)->press = 1;

    /* 保存相对于Button本身按下的坐标位置*/
    WINDATA(data)->cx = ((GdkEventMotion*)event)->x;
    WINDATA(data)->cy = ((GdkEventMotion*)event)->y;

    return FALSE;
}

gboolean release_button (GtkWidget *widget, GdkEventKey *event, gpointer *data) {

    //printf("Release button\n");
    WINDATA(data)->press = 0;
    WINDATA(data)->drag = 0;
    WINDATA(data)->cx = 0;
    WINDATA(data)->cy = 0;
    return FALSE;
}


void listenRelativeEvent(GtkWidget *button, WinData *win ) {

    g_signal_connect ( button, "clicked", G_CALLBACK(mp3play), &win->who );
    g_signal_connect ( button, "enter-notify-event",G_CALLBACK(enter_button), win);
    g_signal_connect ( button, "leave-notify-event",G_CALLBACK(leave_button), win);
    g_signal_connect ( button, "button-press-event",G_CALLBACK(press_button), win);
    g_signal_connect ( button, "button-release-event",G_CALLBACK(release_button), win);
    g_signal_connect(win->window, "motion-notify-event", G_CALLBACK(deal_motion_notify_event), win);

}
