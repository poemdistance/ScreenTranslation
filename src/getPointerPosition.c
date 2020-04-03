#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include "pointer.h"
#include "configControl.h"
#include "windowData.h"
#include "printWithColor.h"

static int _XlibErrorHandler(Display *display, XErrorEvent *event) {
    pred("~~~~~~~~~An error occured detecting the mouse position~~~~~~~~~");
    return True;
}

extern volatile sig_atomic_t SIGTERM_NOTIFY;

/* FROM:https://stackoverflow.com/questions/3585871/how-can-i-get-the-current-mouse-pointer-position-co-ordinates-in-x*/
void *detectPointerPosition ( void *arg ) {

    ConfigData *cd = arg;

    int number_of_screens;
    int i;
    Bool result;
    Window *root_windows;
    Window window_returned;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int mask_return;

    /* XInitThreads(); */ //不要启用这句，程序会崩溃
    Display *display = XOpenDisplay(NULL);

    if ( !display ) return (void*)0;

    XSetErrorHandler(_XlibErrorHandler);
    number_of_screens = XScreenCount(display);

    root_windows = malloc(sizeof(Window) * number_of_screens);

    for (i = 0; i < number_of_screens; i++) {
        root_windows[i] = XRootWindow(display, i);
    }
    while ( 1 ) {

        usleep(1000);

        if ( SIGTERM_NOTIFY ) break;

        for (i = 0; i < number_of_screens; i++) {

            result = XQueryPointer(display, root_windows[i], &window_returned,
                    &window_returned, &root_x, &root_y, &win_x, &win_y,
                    &mask_return);

            if (result == True) {
                break;
            }
        }

        if (result != True) {
            fprintf(stderr, "No mouse found.\n");
            free(root_windows);
            XCloseDisplay(display);
            return NULL;
        }

        cd->pointerx = root_x;
        cd->pointery = root_y;
    }

    pbred ( "detectPointerPosition 退出" );

    free(root_windows);
    XCloseDisplay(display);

    return NULL;
}

/* int main() { */

/*     int x, y; */
/*     detectPointerPosition( NULL ); */
/* } */
