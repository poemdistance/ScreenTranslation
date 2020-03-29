#include <gtk/gtk.h>
#include "settingWindowData.h"
#include "panel.h"
#include "expanduser.h"
#include "printWithColor.h"
#include "configControl.h"
#include "useful.h"
                                
#define LAYOUT_WIDGET_NUM_MAX (10)
#define ICON_MARGIN_START ( 160 )
#define SAVE_BUTTON_MARGIN_START ( 260 )
#define POINTER_PIC_OFFSET (5)
                                            
#define TEXT_WIDTH ( 30 )
#define TEXT_HEIGHT ( 30 )
#define ICON_WIDTH ( 30 )
#define ICON_HEIGHT ( 30 )

Super super = on_configure_event_cb;

static GObject *LayoutWidget[LAYOUT_WIDGET_NUM_MAX] = { '\0' };
static gint LayoutWidgetX[LAYOUT_WIDGET_NUM_MAX] = { 0 };
static gint LayoutWidgetY[LAYOUT_WIDGET_NUM_MAX] = { 0 };
static int layout_widget_num = 0;

static gboolean on_enter_pointer_button_cb ( 
        GtkWidget *widget,
        gpointer data
        ) {

    return TRUE;
}

static gboolean on_press_pointer_button_cb ( 
        GtkWidget *widget,
        gpointer data
        ) {

    return TRUE;
}

static void on_save_button_click_cb ( 
        GtkWidget *saveButton,
        SettingWindowData *swd
        ) {

    IconPositionSettingWindowData *ipswd = swd->iconPositionSettingWindowData;
    gint realOffsetX, realOffsetY;
    gint absoluteX, absoluteY;
    realOffsetX = ipswd->iconOffsetX;
    realOffsetY = ipswd->iconOffsetY;

    pgreen ( "Save icon popup offset: %d %d", realOffsetX, realOffsetY );

    char buf[32];
    writeToConfig ( "Icon-Popup-Offset-X", int2str(realOffsetX, buf) );
    writeToConfig ( "Icon-Popup-Offset-Y",  int2str(realOffsetY, buf));

    for ( int i=0; i<layout_widget_num; i++ ) {
        if ( (GtkWidget*)LayoutWidget[i] == ipswd->icon ) {
            absoluteX = LayoutWidgetX[i];
            absoluteY = LayoutWidgetY[i];
            break;
        }
    }

    writeToConfig ( "Icon-Absolute-X", int2str(absoluteX, buf) );
    writeToConfig ( "Icon-Absolute-Y", int2str(absoluteY, buf) );

    pgreen ( "Save icon absolute X Y: %d %d", absoluteX, absoluteY );
}

/* 调整GtkLayout中各个部件的位置，使其同步窗口伸缩*/
void adjustLayoutWidget ( SettingWindowData *swd ) {

    IconPositionSettingWindowData *ipswd = swd->iconPositionSettingWindowData;
    GtkWidget *window = swd->window;

    int height=0, width=0;
    gtk_window_get_size ( GTK_WINDOW(window), &width, &height );
    int variable = (int)( 47*1.0/100*width-300 );
    int X = 0,  Y = 0;

    GtkWidget *current;

    for ( int i=0; i<layout_widget_num ; i++ ) {

        current = (GtkWidget*)LayoutWidget[i] ;
        if ( current == ipswd->pointer )  {
            X = variable + ICON_MARGIN_START;
            Y = LayoutWidgetY[i];
        }
        else if ( current == ipswd->icon ) {
            /* Do not change following code!*/
            X = variable+ICON_MARGIN_START+ipswd->iconOffsetX;
            Y = LayoutWidgetY[i];
        }
        else if ( current == ipswd->saveButton ) {

            X = variable+SAVE_BUTTON_MARGIN_START;
            Y =  LayoutWidgetY[i];
        }
        else {
            X = variable;
            Y = LayoutWidgetY[i];
        }

        gtk_layout_move ( GTK_LAYOUT(ipswd->layout), current, X, Y);
        LayoutWidgetX[i] = X;
        LayoutWidgetY[i] = Y;
    }

    if ( ipswd->timeoutID ) {
        g_source_remove ( ipswd->timeoutID );
        ipswd->timeoutID = 0;
    }
}

/* 记录下GtkLayout中各部件的原始坐标*/
static void on_entry_size_allocate_cb ( 
        GtkWidget *widget,
        GdkRectangle *alloc,
        SettingWindowData *swd ) {

    IconPositionSettingWindowData *ipswd = swd->iconPositionSettingWindowData;
    char buf[32];
    int i = 0;

    if ( ipswd->come == layout_widget_num ) { return; }
    ipswd->come++;

    for ( i=0; i<layout_widget_num ;i++ ) {
        if ( widget == (GtkWidget*)LayoutWidget[i] ) {
            LayoutWidgetX[i] = alloc->x;
            LayoutWidgetY[i] = alloc->y;
            break;
        }
    }

    /* Restore the related value saved last time*/
    if ( widget == ipswd->icon ) {
        ipswd->iconOffsetX = str2int ( readFromConfig ( "Icon-Popup-Offset-X", buf ) )
            +POINTER_PIC_OFFSET;
        /* pgreen ( "Read icon position, offsetX %s" , buf); */
        ipswd->iconOffsetY = str2int ( readFromConfig ( "Icon-Popup-Offset-Y", buf ) );
        /* pgreen ( "Read icon position, offsetY %s" , buf); */
        LayoutWidgetX[i] = str2int( readFromConfig ( "Icon-Absolute-X", buf ) );
        /* pgreen ( "Read icon position, absoluteX %s" , buf); */
        LayoutWidgetY[i] = str2int( readFromConfig ( "Icon-Absolute-Y", buf ) );
        /* pgreen ( "Read icon position, absoluteY %s" , buf); */
    }
}

/* Resize窗口时的回调函数，用于保存窗口大小，以及调整
 * GtkLayout部件位置*/
static gboolean on_configure_event_child_cb( 
        GtkWindow *window,
        GdkEvent *event,
        SettingWindowData *swd ) {

    /* 关闭原来的回调函数, 改用super调用*/
    if ( swd->configure_event_signal_id ) {
        g_signal_handler_disconnect ( swd->window,
                swd->configure_event_signal_id );
        swd->configure_event_signal_id = 0;
    }

    static int previousWidth = 0;
    int height=0, width=0;
    gtk_window_get_size ( GTK_WINDOW(swd->window), &width, &height );

    /* 仍需保存窗口大小, 需继续调用原来的函数*/
    super ( window, event, swd );

    if ( width == previousWidth )
        return FALSE;

    previousWidth = width;

    adjustLayoutWidget ( swd );

    return FALSE;
}

/* 用于移动icon */
static void on_motion_notify_event ( 
        GtkWidget *window,
        GdkEvent *event,
        SettingWindowData *swd ) {

    IconPositionSettingWindowData *ipswd = swd->iconPositionSettingWindowData;

    if ( ipswd->iconPress && ipswd->iconEnter ) ipswd->iconDrag = TRUE;

    if ( ! ipswd->iconDrag ) return;

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

    /* 指针相对于电脑左上角的坐标*/
    gint px = (int)e->x_root; 
    gint py = (int)e->y_root; 
    gint targetx = px-wx-(int)dwx-ipswd->cx;
    gint targety = py-wy-(int)dwy-ipswd->cy;

    gtk_layout_move ( 
            GTK_LAYOUT(ipswd->layout),
            ipswd->icon, 
            targetx,
            targety);

    int base;
    for ( int i=0; i<layout_widget_num ;i++ ) {
        if ( ipswd->pointer == (GtkWidget*)LayoutWidget[i] ) {
            base = i; break;
        }
    }

    for ( int i=0; i<layout_widget_num ;i++ ) {
        if ( ipswd->icon == (GtkWidget*)LayoutWidget[i] ) {
            LayoutWidgetX[i] = targetx;
            LayoutWidgetY[i] = targety;

            /* icon 相对于 Text 的位置*/
            ipswd->iconOffsetX = targetx-LayoutWidgetX[base]-POINTER_PIC_OFFSET;
            ipswd->iconOffsetY = targety-LayoutWidgetY[base]-15;
            break;
        }
    }

    gtk_widget_show ( ipswd->icon );
}

static void on_enter_notify_cb ( 
        GtkWidget *icon,
        GdkEvent *event,
        SettingWindowData *swd ) {

    IconPositionSettingWindowData *ipswd = swd->iconPositionSettingWindowData;
    ipswd->iconEnter = TRUE;
}

gboolean leave_icon (GtkWidget *widget, GdkEventKey *event, SettingWindowData *swd) {

    IconPositionSettingWindowData *ipswd  = swd->iconPositionSettingWindowData;
    ipswd->iconEnter = FALSE;
    return TRUE;
}

gboolean press_icon (GtkWidget *widget, GdkEventKey *event, SettingWindowData *swd ) {

    IconPositionSettingWindowData *ipswd  = swd->iconPositionSettingWindowData;
    ipswd->iconPress = TRUE;

    /* 保存相对于icon本身按下的坐标位置*/
    ipswd->cx = ((GdkEventMotion*)event)->x;
    ipswd->cy = ((GdkEventMotion*)event)->y;

    return TRUE;
}

gboolean release_icon (GtkWidget *widget, GdkEventKey *event, SettingWindowData *swd) {

    IconPositionSettingWindowData *ipswd  = swd->iconPositionSettingWindowData;
    ipswd->iconPress = FALSE;
    ipswd->iconDrag = FALSE;
    ipswd->cx = 0;
    ipswd->cy = 0;
    return TRUE;
}
void iconPositionSetting ( SettingWindowData *settingWindowData ) {

    SettingWindowData *swd = settingWindowData;
    static IconPositionSettingWindowData ipswd;
    swd->iconPositionSettingWindowData = &ipswd;

    /* Remove all the previous widgets in container*/
    gtk_container_forall (
            (GtkContainer*)swd->contentScrollWindow,
            remove_widget,
            (settingWindowData->contentScrollWindow)
            );

    GtkBuilder *builder = gtk_builder_new_from_file ( 
            expanduser("/home/$USER/.stran/icon_position_setting.ui") );

    int i = -1;
    layout_widget_num = 0;
    ipswd.come = 0;

    GObject *layout = gtk_builder_get_object ( builder, "layout" );
    GObject *entry = LayoutWidget[++i] = gtk_builder_get_object ( builder, "entry" );
    GObject *entry1 = LayoutWidget[++i] = gtk_builder_get_object ( builder, "entry1" );
    GObject *entry2 = LayoutWidget[++i] = gtk_builder_get_object ( builder, "entry2" );
    GObject *pointer = LayoutWidget[++i] = gtk_builder_get_object ( builder, "pointer" );
    GObject *icon = LayoutWidget[++i] = gtk_builder_get_object ( builder, "icon" );
    GObject *saveButton = LayoutWidget[++i] = gtk_builder_get_object ( builder, "saveButton" );

    layout_widget_num = i + 1;


    ipswd.layout = (GtkWidget*)layout;
    ipswd.entry = (GtkWidget*)entry;
    ipswd.entry1 = (GtkWidget*)entry1;
    ipswd.entry2 = (GtkWidget*)entry2;
    ipswd.pointer = (GtkWidget*)pointer;
    ipswd.icon =  (GtkWidget*)icon;
    ipswd.saveButton = (GtkWidget*)saveButton;

    GtkWidget *pointerImage = gtk_image_new_from_pixbuf(
            gdk_cursor_get_image (
                gdk_cursor_new_from_name ( 
                    gdk_display_get_default(),
                    "default"
                    )
                )
            );

    gtk_button_set_image ( GTK_BUTTON(pointer), pointerImage );

    gtk_widget_set_hexpand ( (GtkWidget*)layout, TRUE );
    gtk_widget_set_vexpand ( (GtkWidget*)layout, TRUE );
    gtk_widget_set_margin_start ( (GtkWidget*)layout, 0 );
    gtk_widget_set_margin_end ( (GtkWidget*)layout, 0 );

    gtk_container_add ( GTK_CONTAINER(swd->contentScrollWindow), (GtkWidget*)layout );

    g_signal_connect ( swd->contentScrollWindow, "motion_notify_event", 
            G_CALLBACK(on_motion_notify_event), swd);

    g_signal_connect ( ipswd.icon, "enter-notify-event", 
            G_CALLBACK(on_enter_notify_cb), swd);
    g_signal_connect ( ipswd.icon, "leave-notify-event",
            G_CALLBACK(leave_icon), swd);
    g_signal_connect ( ipswd.icon, "button-press-event",
            G_CALLBACK(press_icon), swd);
    g_signal_connect ( ipswd.icon, "button-release-event",
            G_CALLBACK(release_icon), swd);
    g_signal_connect ( swd->window, "configure-event",
            G_CALLBACK(on_configure_event_child_cb), settingWindowData );
    g_signal_connect ( ipswd.saveButton, "clicked", 
            G_CALLBACK(on_save_button_click_cb), settingWindowData );

    /* 接收相关信号以禁止按钮轮廓被显示出来*/
    g_signal_connect ( ipswd.pointer, "enter-notify-event", 
            G_CALLBACK(on_enter_pointer_button_cb), NULL);
    g_signal_connect ( ipswd.pointer, "button-press-event",
            G_CALLBACK(on_press_pointer_button_cb), NULL);

    ipswd.timeoutID = 
        g_timeout_add ( 100, (int (*)())adjustLayoutWidget, swd );

    for ( i=0; i<layout_widget_num; i++ ) {
        g_signal_connect ( LayoutWidget[i], "size-allocate", 
                G_CALLBACK(on_entry_size_allocate_cb), settingWindowData);
    }
}
