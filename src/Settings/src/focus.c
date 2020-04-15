#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include "focus.h"
#include "printWithColor.h"

/* 本函数代码借鉴自xdotool部分源码*/
int focusRequest( SettingWindowData *swd ) {

    GtkWidget *window = swd->window;

    if ( ! window ) {
        pred ( "NULL WINDOW" );
        return FALSE;
    }

    /* Get window id of x11*/
    GdkWindow *gw = gtk_widget_get_window ( GTK_WIDGET ( window ) );
    Window wid = gdk_x11_window_get_xid ( gw );

    /* Get window's attributes*/
    XWindowAttributes wattr;
    XInitThreads();
    Display *dpy = XOpenDisplay (NULL);
    if ( !dpy )
        return FALSE;

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

    if ( ret == 0 ) {
        pred("窗口聚焦请求失败(focusRequest)");
    }

    XCloseDisplay(dpy);

    return TRUE;
}
