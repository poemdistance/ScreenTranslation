#include "common.h"
#include <assert.h>
#include <poll.h>
#include <X11/extensions/Xfixes.h>
#include "cleanup.h"
#include "shmData.h"

static int SIGTERM_SIGNAL = 0;

void WatchSelection(Display *display, const char *bufname, Arg *arg);

static void exitNotify ( ) {
    SIGTERM_SIGNAL = 1;
}

void checkSelectionChanged ( void *arg )
{
    Display *display = XOpenDisplay(NULL);
    unsigned long color = BlackPixel(display, DefaultScreen(display));
    XCreateSimpleWindow(display, DefaultRootWindow(display), 0,0, 1,1, 0, color, color);

    struct sigaction sa;
    sa.sa_handler = exitNotify;
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGTERM, &sa, NULL) != 0 ) {
        pred ( "sigaction exec failed in checkSelectionChanged.c" );
        pred ( "function: checkSelectionChanged" );
        perror("sigaction");
        XCloseDisplay ( display );
        quit ( arg );
    }

    if ( sigaction(SIGINT, &sa, NULL) == -1) {
        pred("sigaction err(checkSelectionChanged -> SIGTERM)");
        perror("sigaction");
        XCloseDisplay ( display );
        quit ( arg );
    }

    const char buf[] = "PRIMARY";
    WatchSelection ( display, buf, arg);
}

void WatchSelection(Display *display, const char *bufname, Arg *arg)
{
    ShmData *sd = arg->sd;
    int event_base, error_base;
    XEvent event;
    Atom bufid = XInternAtom(display, bufname, False);

    int fd;

    fd = XConnectionNumber ( display );

    assert( XFixesQueryExtension(display, &event_base, &error_base) );
    XFixesSelectSelectionInput(display, DefaultRootWindow(display), bufid, XFixesSetSelectionOwnerNotifyMask);

    struct pollfd pfd;
    int timeout = 200;
    pfd.fd = fd;
    pfd.events = POLLIN|POLLPRI;

    pbmag ( "启动 checkSelectionChanged" );

    while ( 1 ) {

        poll ( &pfd, 1, timeout );

        memset ( &event, '\0', sizeof(event) );
        while(XPending(display))
            XNextEvent(display, &event);

        if (event.type == event_base + XFixesSelectionNotify &&
                ((XFixesSelectionNotifyEvent*)&event)->selection == bufid) {

            if ( sd->shmaddr_selection[0] != '1' ) {

                sd->shmaddr_selection[0] = '1';
                pgreen("Selection change: write finish flag: 1");
            }
        }

        if ( SIGTERM_SIGNAL ) break;
    }

    pbcyan ( "剪贴板监测程序退出" );
    XCloseDisplay ( display );
}
