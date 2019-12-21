#include "common.h"
#include "windowData.h"
#include "newWindow.h"
#include "calibration.h"
#include "audio.h"
#include "fitting.h" /* notExist()*/

void writeAudioButtonInfo ( int l, int x, int y ) {

    char cmd[140] = "bash ";
    char num[20] = { '\0' };
    char path[] = "/home/$USER/.stran/buttonPositionDataCtl.sh ";
    char file[] = "/home/$USER/.stran/"AUDIO_BASE_NAME".data";

    if ( notExist ( path ) ) {
        pbred ( "%s not exist ( writeAudioButtonInfo )", path );
        return;
    }

    if ( notExist ( file ) ) {
        pbred ( "%s not exist, creating it...", file );
        fclose ( fopen ( file, "w" ) );
    }

    sprintf(num, "%d %d %d ", l, x, y );
    system ( strcat ( strcat ( strcat ( cmd, path ), num ), file ) );

}

void writeWinSizeInfo ( int linelen, int width, int linenum, int height  ) {

    char cmd[140] = "bash ";
    char num[20] = { '\0' };
    char path[] = "/home/$USER/.stran/winSizeDataCtl.sh ";
    char file[] = "/home/$USER/.stran/"WIN_SIZE_BASE_NAME".data";

    sprintf(num, "%d %d %d %d ", linelen, width, linenum, height );
    system ( strcat ( strcat ( strcat ( cmd, path ), num ), file ) );
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
            WINDATA(data)->ox, WINDATA(data)->oy);
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
