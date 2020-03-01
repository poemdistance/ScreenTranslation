#include "common.h"
#include <assert.h>
#include <X11/extensions/Xfixes.h>
#include "cleanup.h"

extern char *shmaddr_selection;
extern int action;

static int SIGTERM_SIGNAL = 0;

void WatchSelection(Display *display, const char *bufname);

static void exitNotify ( ) {
    SIGTERM_SIGNAL = 1;
}

void checkSelectionChanged(int writefd, int readfd)
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
        quit();
    }

    sa.sa_handler =  exitNotify;
    sigemptyset ( &sa.sa_mask );
    if ( sigaction(SIGINT, &sa, NULL) == -1) {
        pred("sigaction err(checkSelectionChanged -> SIGTERM)");
        perror("sigaction");
        XCloseDisplay ( display );
        quit();
    }

    const char buf[] = "PRIMARY";
    WatchSelection ( display, buf);
}

void WatchSelection(Display *display, const char *bufname)
{
    int event_base, error_base;
    XEvent event;
    Atom bufid = XInternAtom(display, bufname, False);

    int fd;
    fd_set fds;
    struct timeval tv;

    fd = ConnectionNumber ( display );

    assert( XFixesQueryExtension(display, &event_base, &error_base) );
    XFixesSelectSelectionInput(display, DefaultRootWindow(display), bufid, XFixesSetSelectionOwnerNotifyMask);

    while ( 1 ) {

        FD_ZERO ( &fds );
        FD_SET ( fd, &fds );
        tv.tv_sec = 0;
        tv.tv_usec = 100000;

        select ( fd+1, &fds, NULL, NULL, &tv );

        memset ( &event, '\0', sizeof(event) );
        while(XPending(display))
            XNextEvent(display, &event);

        /* printf("Waiting next event (WatchSelection)\n"); */
        /* XNextEvent ( display, &event ); */

        if (event.type == event_base + XFixesSelectionNotify &&
                ((XFixesSelectionNotifyEvent*)&event)->selection == bufid) {

            if ( shmaddr_selection[0] != '1' ) {

                shmaddr_selection[0] = '1';
                pred("Selection change: write finish flag: 1");
            }
        }

        if ( SIGTERM_SIGNAL ) break;
    }

    pgreen ( "checkSelectionChanged exit" );
    XCloseDisplay ( display );
}
