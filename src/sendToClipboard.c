#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include <stdlib.h>

void err_exit(char *arg) {
    fprintf(stderr, "%s", arg);
    exit(1);
}

void send_no(Display *dpy, XSelectionRequestEvent *xsre);

void send_utf8 
( Display *dpy, XSelectionRequestEvent *xsre, Atom utf8, char *arg);

int main (int argc, char **argv) 
{
    Display *dpy;
    Window owner, root;
    int screen;
    Atom sel, utf8;
    XEvent ev;
    XSelectionRequestEvent *sev;

    dpy = XOpenDisplay(NULL);
    if ( ! dpy ) {
        fprintf(stderr, "Could not open X display\n");
        exit(1);
    }

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);

    owner = XCreateSimpleWindow(dpy, root, -10, -10, 1, 1, 0,0,0);

    sel = XInternAtom(dpy, "CLIPBOARD", False);
    utf8 = XInternAtom(dpy, "UTF8_STRING", CurrentTime);

    Window win = XSetSelectionOwner(dpy, sel, owner, CurrentTime);
    if ( win == None )
        err_exit("set selection owner fail");
    else
        printf("set selection owner successful 0x%lx\n", win);

    while(1) {

        XNextEvent(dpy, &ev);

        switch ( ev.type ) {
            case SelectionClear:
                printf("Lost selection ownership\n");
                exit(1);
                XCloseDisplay(dpy);
                break;
            case SelectionRequest:
                sev = (XSelectionRequestEvent*)&ev.xselectionrequest;
                printf("Requestor: 0x%lx\n", sev->requestor);
                if(sev->target != utf8 || sev->property == None) {
                    send_no(dpy, sev);
                    XFlush(dpy);
                    XCloseDisplay(dpy);
                    return 0;
                }
                else {
                    send_utf8(dpy, sev, utf8, argv[1]);
                    XFlush(dpy);
                    XCloseDisplay(dpy);
                    return 0;
                }
                break;
            default: 
                break;
        }
    }
}

void send_no(Display *dpy, XSelectionRequestEvent *xsre) {
    XSelectionEvent xse;
    char *atomnum;

    atomnum = XGetAtomName(dpy, xsre->target);
    printf("Denying request of type %s\n", atomnum);

    if ( atomnum )
        XFree(atomnum);

    xse.type = SelectionNotify;
    xse.requestor = xsre->requestor;
    xse.selection = xsre->selection;
    xse.target = xsre->target;
    xse.property = None;
    xse.time = xsre->time;

    XSendEvent( dpy, xsre->requestor, True, NoEventMask, (XEvent*)&xse);
}

void send_utf8 
( Display *dpy, XSelectionRequestEvent *xsre, Atom utf8, char *arg) {

    XSelectionEvent xse;
    char *atomname;

    atomname = XGetAtomName(dpy, xsre->property);
    printf("Sending data to window 0x%lx , property %s\n", xsre->requestor, atomname);

    if ( ! atomname )
        XFree(atomname);

    XChangeProperty(dpy, xsre->requestor, xsre->property, utf8, 8, PropModeAppend, (unsigned char*)arg, strlen((const char*)arg));

    xse.type = SelectionNotify;
    xse.requestor = xsre->requestor;
    xse.selection = xsre->selection;
    xse.target = xsre->target;
    xse.property = xsre->property;
    xse.time = xsre->time;

    XSendEvent( dpy, xsre->requestor, True, NoEventMask, (XEvent*)&xse );
}
