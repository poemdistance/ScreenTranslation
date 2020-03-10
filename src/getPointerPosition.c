#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include "pointer.h"

static int _XlibErrorHandler(Display *display, XErrorEvent *event) {
    fprintf(stderr, "An error occured detecting the mouse position\n");
    return True;
}

/* FROM:https://stackoverflow.com/questions/3585871/how-can-i-get-the-current-mouse-pointer-position-co-ordinates-in-x*/
int getPointerPosition( int *x, int *y ) {

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

    if ( !display ) 
        return FALSE;

    XSetErrorHandler(_XlibErrorHandler);
    number_of_screens = XScreenCount(display);
    /* fprintf(stderr, "There are %d screens available in this X session\n", number_of_screens); */
    root_windows = malloc(sizeof(Window) * number_of_screens);
    for (i = 0; i < number_of_screens; i++) {
        root_windows[i] = XRootWindow(display, i);
    }
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
        XCloseDisplay(display);
        return -1;
    }
    *x = root_x;
    *y = root_y;

    /* printf("Mouse is at (%d,%d)\n", root_x, root_y); */

    free(root_windows);
    XCloseDisplay(display);

    return 0;
}
