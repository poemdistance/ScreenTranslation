#include <gtk/gtk.h>
#include "settingWindowData.h"
#include "panel.h"
#include "expanduser.h"
#include "printWithColor.h"
#include "configControl.h"
#include "useful.h"

#define LAYOUT_WIDGET_NUM_MAX (10)
#define SAVE_BUTTON_MARGIN_START ( 280 )

static GObject *LayoutWidget[LAYOUT_WIDGET_NUM_MAX] = { '\0' };
static gint LayoutWidgetX[LAYOUT_WIDGET_NUM_MAX] = { 0 };
static gint LayoutWidgetY[LAYOUT_WIDGET_NUM_MAX] = { 0 };
static int layout_widget_num = 0;

static void updateLayout  (
        GtkSwitch *widget, 
        gboolean hideTitleBar,
        SettingWindowData *swd
        );

static void on_save_button_click_cb ( 
        GtkWidget *button,
        SettingWindowData *swd
        ) {

    WinPosSettingWindowData *wpswd = 
        swd->winPosSettingWindowData;

    char buf[16];
    writeToConfig ( "Pointer-Offset-X", int2str(wpswd->pointerOffsetX, buf) );
    pblue ( "Write buf: %s", buf );
    writeToConfig ( "Pointer-Offset-Y", int2str(wpswd->pointerOffsetY, buf) );
    pblue ( "Write buf: %s", buf );
    writeToConfig ( "Pointer-Absolute-X", int2str(wpswd->pointerAbsoluteX, buf) );
    pblue ( "Write buf: %s", buf );
    writeToConfig ( "Pointer-Absolute-Y", int2str(wpswd->pointerAbsoluteY, buf) );
    pblue ( "Write buf: %s", buf );
}

static void on_list_box_size_allocate_cb ( 
        GtkWidget *widget,
        GdkRectangle *alloc,
        SettingWindowData *swd
        ) {

    WinPosSettingWindowData *wpswd =
        swd->winPosSettingWindowData;

    /* pbcyan ( "list box %d %d", alloc->width, alloc->height ); */

    wpswd->listBoxHeight = alloc->height;
}

static void on_layout_widget_size_allocate_cb ( 
        GtkWidget *widget,
        GdkRectangle *alloc,
        SettingWindowData *swd
        ) {

    WinPosSettingWindowData *wpswd =
        swd->winPosSettingWindowData;

    char buf[32];
    int i = 0;

    if ( wpswd->come == layout_widget_num+1 ) { return; }
    wpswd->come++;

    for ( i=0; i<=layout_widget_num ;i++ ) {
        if ( widget == (GtkWidget*)LayoutWidget[i] ) {
            LayoutWidgetX[i] = alloc->x;
            LayoutWidgetY[i] = alloc->y;
            break;
        }
    }

    if ( wpswd->come == layout_widget_num + 1 )
        updateLayout ( NULL, str2bool(readFromConfig("Hide-Header-Bar-Pref", buf)), swd );


    if ( widget == wpswd->pointer ) {

        pmag ( "恢复配置" );

        LayoutWidgetY[i] = str2int(readFromConfig ( "Pointer-Absolute-Y", buf ));
        pyellow ( "Buf:%s", buf );
        LayoutWidgetX[i] = str2int(readFromConfig ( "Pointer-Absolute-X", buf ));
        pyellow ( "Buf:%s", buf );
        wpswd->pointerOffsetX = str2int(readFromConfig ( "Pointer-Offset-X", buf ));
        pyellow ( "Buf:%s", buf );
        wpswd->pointerOffsetY = str2int(readFromConfig ( "Pointer-Offset-Y", buf ));
        pyellow ( "Buf:%s", buf );

        wpswd->pointerAbsoluteX = LayoutWidgetX[i];
        wpswd->pointerAbsoluteY = LayoutWidgetY[i];
    }
}

static void adjustLayoutWidget ( SettingWindowData *swd ) {

    WinPosSettingWindowData *wpswd =
        swd->winPosSettingWindowData;
    GtkWidget *window = swd->window;

    int height=0, width=0;
    gtk_window_get_size ( GTK_WINDOW(window), &width, &height );
    int variable = (int)( 47*1.0/100*width-350 );
    int X = 0,  Y = 0;

    GtkWidget *current;

    for ( int i=0; i<=layout_widget_num; i++ ) {

        current = (GtkWidget*)LayoutWidget[i];

        if ( current == wpswd->pointer ) {
            X = variable+wpswd->pointerOffsetX;
            Y = LayoutWidgetY[i];
        }
        else if ( current == wpswd->saveButton ) {
            X = variable+SAVE_BUTTON_MARGIN_START;
            Y = LayoutWidgetY[i];
        }
        else {
            X = variable;
            Y = LayoutWidgetY[i];
        }

        gtk_layout_move ( 
                GTK_LAYOUT( wpswd->layout ),
                current, X,Y);

        /* Update LayoutWidget position*/
        LayoutWidgetX[i] = X;
        LayoutWidgetY[i] = Y;
    }

    if ( wpswd->timeoutID_adjustLayoutWidget ) {
        g_source_remove ( wpswd->timeoutID_adjustLayoutWidget );
        wpswd->timeoutID_adjustLayoutWidget = 0;
    }
}

static gboolean on_configure_event_child_cb ( 
        GtkWidget *window, 
        GdkEvent *event,
        SettingWindowData *swd
        ) {

    static int previousWidth = 0;
    int height=0, width=0;
    gtk_window_get_size ( GTK_WINDOW(swd->window), &width, &height );

    if ( width == previousWidth )
        return FALSE;

    previousWidth = width;

    adjustLayoutWidget ( swd );

    return FALSE;
}

static void on_layout_size_allocate_cb ( 
        GtkWidget *layout, 
        GtkAllocation *alloc,
        SettingWindowData *swd
        ) {

    WinPosSettingWindowData *wpswd  =
        swd->winPosSettingWindowData;

    wpswd->layoutWidth = alloc->width;
    wpswd->layoutHeight = alloc->height;
}

static void on_motion_notify_event ( 
        GtkWidget *window,
        GdkEvent *event,
        SettingWindowData *swd ) {

    WinPosSettingWindowData *wpswd  =
        swd->winPosSettingWindowData;


    if ( wpswd->pointerPress && wpswd->pointerEnter ) wpswd->pointerDrag = TRUE;

    if ( ! wpswd->pointerDrag ) return;

    GdkEventButton *e = (GdkEventButton*)event;

    /* 指针相对于窗口左上角的坐标*/
    gint wx, wy;
    gdk_window_get_position ( 
            gtk_widget_get_window ( swd->window ),
            &wx, &wy);

    /* contentScrollWindow相对于父窗口的坐标*/
    gdouble dwx, dwy;
    gdk_window_coords_to_parent ( 
            gtk_widget_get_window(swd->contentScrollWindow),
            0, 0, &dwx, &dwy);

    /* pmag ( "content window %f %f", dwx, dwy ); */

    double ddwx, ddwy;
    gdk_window_coords_to_parent ( 
            gtk_widget_get_window(wpswd->layout),
            0, 0, &ddwx, &ddwy);

    /* pmag ( "layout %f %f", ddwx, ddwy ); */

    gint w, h;
    gtk_window_get_size ( 
            (GtkWindow*)swd->window,
            &w, &h );

    /* 指针相对于电脑左上角的坐标*/
    gint px = (int)e->x_root; 
    gint py = (int)e->y_root; 

    gint targetx = px-wx-(int)dwx-(int)ddwx-wpswd->cx;
    gint targety = py-wy-(int)dwy-(int)ddwx-wpswd->cy-wpswd->listBoxHeight;

    targetx = targetx < 1 ? 0 : targetx;
    targety = targety < 0 ? 0 : targety;

    /* printf("%d %d %d %d\n", targetx, targety, wpswd->layoutWidth, wpswd->layoutHeight); */
    if ( targetx+30 > wpswd->layoutWidth ||targety+30 >wpswd->layoutHeight )
        return;

    gtk_layout_move ( 
            GTK_LAYOUT(wpswd->layout),
            wpswd->pointer, 
            targetx,
            targety);


    wpswd->pointerAbsoluteX = targetx;
    wpswd->pointerAbsoluteY = targety;

    int base;
    for ( int i=0; i<=layout_widget_num ;i++ ) {
        if ( wpswd->currentImage == (GtkWidget*)LayoutWidget[i] ) {
            base = i; break;
        }
    }

    for ( int i=0; i<=layout_widget_num ;i++ ) {
        if ( wpswd->pointer == (GtkWidget*)LayoutWidget[i] ) {
            LayoutWidgetX[i] = targetx;
            LayoutWidgetY[i] = targety;

            /* pointer 相对于 Text 的位置 */
            wpswd->pointerOffsetX = targetx-LayoutWidgetX[base];
            wpswd->pointerOffsetY = targety-LayoutWidgetY[base];
            break;
        }
    }
    gtk_widget_show ( wpswd->pointer );
}

static gboolean on_leave_pointer_cb (
        GtkWidget *widget,
        GdkEvent *event,
        SettingWindowData *swd) {

    WinPosSettingWindowData *wpswd  =
        swd->winPosSettingWindowData;

    wpswd->pointerEnter = FALSE;
    return TRUE;
}
static gboolean on_enter_pointer_button_cb ( 
        GtkWidget *widget,
        GdkEvent *event,
        SettingWindowData *swd
        ) {

    WinPosSettingWindowData *wpswd  =
        swd->winPosSettingWindowData;
    wpswd->pointerEnter = TRUE;
    return TRUE;
}

static gboolean on_release_pointer_button_cb ( 
        GtkWidget *widget,
        GdkEvent *event,
        SettingWindowData *swd
        ) {
    WinPosSettingWindowData *wpswd  =
        swd->winPosSettingWindowData;
    wpswd->pointerPress = FALSE;
    wpswd->pointerDrag = FALSE;
    wpswd->cx = 0;
    wpswd->cy = 0;

    return TRUE;
}
static gboolean on_press_pointer_button_cb ( 
        GtkWidget *widget,
        GdkEvent *event,
        SettingWindowData *swd
        ) {
    WinPosSettingWindowData *wpswd  =
        swd->winPosSettingWindowData;
    wpswd->pointerPress = TRUE;
    /* 保存相对于icon本身按下的坐标位置*/
    wpswd->cx = ((GdkEventMotion*)event)->x;
    wpswd->cy = ((GdkEventMotion*)event)->y;
    return TRUE;
}

/* 根据hide-header-bar开关的开启状态切换显示的图片*/
static void updateLayout  (
        GtkSwitch *widget, 
        gboolean hideTitleBar,
        SettingWindowData *swd
        ) {

    WinPosSettingWindowData *wpswd = swd->winPosSettingWindowData;

    if ( hideTitleBar ) {
        gtk_widget_hide ( wpswd->imageHasTitleBar );
        gtk_widget_queue_draw ( wpswd->layout );
        gtk_widget_show ( wpswd->imageNoTitleBar );
        wpswd->currentImage = wpswd->imageNoTitleBar;
    }
    else {
        gtk_widget_hide ( wpswd->imageNoTitleBar );
        gtk_widget_queue_draw ( wpswd->layout );
        gtk_widget_show ( wpswd->imageHasTitleBar );
        wpswd->currentImage = wpswd->imageHasTitleBar;
    }
}

static void on_switch_state_change ( 
        GtkSwitch *widget,
        gboolean state,
        char *prefName ) {

    char buf[64];
    strcpy ( buf, prefName );
    strcat ( buf, "-Pref" );

    writeToConfig ( buf, bool2str(state) );

    gtk_switch_set_state ( widget, state );
}

void addPrefToListBox ( 
        char (*prefValue)[PREF_CONTENT_LEN],
        char (*prefName)[PREF_CONTENT_LEN],
        SettingWindowData *swd ) {

    WinPosSettingWindowData *wpswd = swd->winPosSettingWindowData;
    GtkListBox *list = (GtkListBox*)wpswd->listBox;
    GtkWidget *grid;
    GtkWidget *box;
    GtkWidget *label;
    GtkWidget *switchButton;

    for ( int i=0; i<MAX_PREF_NUM; i++ ) {

        if ( *prefValue[i] ) {

            grid = gtk_grid_new();
            gtk_widget_set_size_request ( grid, -1, LISTBOX_ROW_HEIGHT );

            /* 快捷键名称*/
            box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_widget_set_size_request ( box, 225, -1 );

            label = gtk_label_new ( prefName[i] );
            gtk_grid_attach ( GTK_GRID(grid), box, 0, 0, 1, 1 );
            gtk_box_pack_start ( GTK_BOX(box), label, 1, 1, 1 );
            gtk_widget_set_vexpand ( label, TRUE );
            gtk_widget_set_hexpand ( label, TRUE );
            gtk_widget_set_margin_start ( label, MARGIN_START );
            gtk_widget_set_halign ( label, GTK_ALIGN_START );

            /* gtk_grid_attach ( GTK_GRID(grid), gtk_separator_new(GTK_ORIENTATION_VERTICAL), 1, 0, 1, 1 ); */

            /* 快捷键键值*/
            box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
            gtk_widget_set_size_request ( box, 145, -1 );
            switchButton = gtk_switch_new();
            gtk_switch_set_state ( GTK_SWITCH(switchButton), str2bool(prefValue[i]) );
            gtk_widget_set_margin_end ( label, MARGIN_END );
            gtk_grid_attach ( GTK_GRID(grid), box, 1, 0, 1, 1 );
            gtk_box_pack_start ( GTK_BOX(box), switchButton, 1, 1, 1 );

            g_signal_connect ( switchButton, "state-set", 
                    G_CALLBACK(on_switch_state_change), prefName[i]);

            if ( strcmp ( prefName[i], "Hide-Header-Bar" ) == 0 )
                g_signal_connect ( switchButton, "state-set", 
                        G_CALLBACK(updateLayout), swd);

            gtk_widget_set_vexpand ( switchButton, FALSE );
            gtk_widget_set_hexpand ( switchButton, FALSE );
            gtk_widget_set_halign ( switchButton, GTK_ALIGN_CENTER );
            gtk_widget_set_valign ( switchButton, GTK_ALIGN_CENTER );

            gtk_grid_set_column_spacing ( GTK_GRID(grid), 100 );

            gtk_list_box_insert ( GTK_LIST_BOX(list), grid, -1 );
        }
    }

    gtk_widget_show_all ( swd->contentScrollWindow );
}


void windowPosSetting ( SettingWindowData *swd ) {

    static WinPosSettingWindowData wpswd;
    swd->winPosSettingWindowData = &wpswd;
    memset ( &wpswd, '\0', sizeof(wpswd) );

    wpswd.come = 0;

    /* Remove all the previous widgets in container*/
    gtk_container_forall (
            (GtkContainer*)swd->contentScrollWindow,
            remove_widget,
            (swd->contentScrollWindow));

    /* GTK BOX*/
    GtkWidget *box = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 ); 
    wpswd.box = box;
    gtk_container_add ( GTK_CONTAINER(swd->contentScrollWindow), box );
    gtk_widget_set_hexpand ( box, TRUE  );
    gtk_widget_set_vexpand ( box, TRUE  );
    gtk_widget_set_margin_start(box, MARGIN_START );
    gtk_widget_set_margin_end(box, MARGIN_END );
    gtk_widget_set_margin_top(box, MARGIN_TOP );

    /* GTK LIST BOX */
    GtkWidget *listBox = gtk_list_box_new();
    wpswd.listBox = listBox;
    gtk_box_pack_start ( GTK_BOX(box), listBox, 0,0,0 );
    gtk_widget_set_halign ( listBox, GTK_ALIGN_CENTER );

    /* 读取所有窗口配置信息和配置名*/
    static char prefName[MAX_PREF_NUM][PREF_CONTENT_LEN];
    static char prefValue[MAX_PREF_NUM][PREF_CONTENT_LEN];
    memset ( prefName, '\0', sizeof(prefName) );
    memset ( prefValue, '\0', sizeof(prefValue) );
    /* readFromConfigByKeyword ( prefValue, "Pref" ); */
    readFromConfigByKeyword ( prefValue, "Hide-Header-Bar-Pref" );
    readNameByKeyword ( prefName, "Pref" );

    /* 添加窗口配置信息到listBox*/
    addPrefToListBox ( prefValue, prefName, swd ); 


    /* 加载ui配置文件*/
    GtkBuilder *builder = gtk_builder_new_from_file ( 
            expanduser("/home/$USER/.stran/window-position.ui"));

    int i = -1;
    GObject *layout = gtk_builder_get_object ( builder, "layout" );
    GObject *imageHasTitleBar = LayoutWidget[++i] = gtk_builder_get_object ( builder, "image-has-title-bar" );
    GObject *imageNoTitleBar = LayoutWidget[++i] = gtk_builder_get_object ( builder, "image-no-title-bar" );
    GObject *saveButton = LayoutWidget[++i] = gtk_builder_get_object ( builder, "saveButton" );
    GObject *pointer = LayoutWidget[++i] = gtk_builder_get_object ( builder, "pointer" );

    layout_widget_num = i;

    /* 保存部件到结构体*/
    wpswd.layout = (GtkWidget*)layout;
    wpswd.pointer = (GtkWidget*)pointer;
    wpswd.imageHasTitleBar = (GtkWidget*)imageHasTitleBar;
    wpswd.imageNoTitleBar = (GtkWidget*)imageNoTitleBar;
    wpswd.saveButton = (GtkWidget*)saveButton;

    inline void disable_selectable_activatable( GtkWidget *widget, gpointer data ) {
        gtk_list_box_row_set_selectable ( GTK_LIST_BOX_ROW(widget), false );
        gtk_list_box_row_set_activatable ( GTK_LIST_BOX_ROW(widget), false );
    }
    for ( i=0; i<=layout_widget_num; i++ ) {
        g_signal_connect ( LayoutWidget[i], "size-allocate", 
                G_CALLBACK(on_layout_widget_size_allocate_cb), swd);
    }

    gtk_box_pack_start ( GTK_BOX(box), (GtkWidget*)layout, 1, 1, 1 );

    /* Set all listBoxRow unSeletable and unActivatable*/
    gtk_container_forall ( 
            GTK_CONTAINER(listBox),
            disable_selectable_activatable,
            NULL);

    gtk_widget_show_all ( swd->contentScrollWindow );

    g_signal_connect ( pointer, "enter-notify-event", 
            G_CALLBACK(on_enter_pointer_button_cb), swd);
    g_signal_connect ( pointer, "button-press-event",
            G_CALLBACK(on_press_pointer_button_cb), swd);
    g_signal_connect ( pointer, "leave-notify-event",
            G_CALLBACK(on_leave_pointer_cb), swd);
    g_signal_connect ( pointer, "button-release-event",
            G_CALLBACK(on_release_pointer_button_cb), swd);
    g_signal_connect ( swd->contentScrollWindow, "motion_notify_event", 
            G_CALLBACK(on_motion_notify_event), swd);

    g_signal_connect ( layout, "size-allocate", 
            G_CALLBACK(on_layout_size_allocate_cb), swd);
    g_signal_connect ( swd->window, "configure-event",
            G_CALLBACK(on_configure_event_child_cb), swd);

    wpswd.timeoutID_adjustLayoutWidget = 
        g_timeout_add ( 100, (int (*)())adjustLayoutWidget, swd );

    g_signal_connect ( listBox, "size-allocate", 
            G_CALLBACK(on_list_box_size_allocate_cb), swd);

    g_signal_connect ( saveButton, "clicked",
            G_CALLBACK(on_save_button_click_cb), swd);
}
