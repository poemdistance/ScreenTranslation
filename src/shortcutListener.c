#include <stdio.h>
#include <poll.h>
#include <X11/Xlib.h>
#include <X11/keysym.h> /* KeySym*/
#include <X11/Xutil.h> /* XLookupString*/
#include <X11/X.h> /* Some mask (ContrlMask...)*/
#include <X11/XKBlib.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/signal.h>
#include <string.h>
#include "printWithColor.h"
#include "shortcutListener.h"
#include "configControl.h"
#include "sharedMemory.h"
#include "common.h"
#include "useful.h"
#include "cleanup.h"

extern char modifier[7][10];
extern int modifier2maskTable[7];

extern char numlock_mask;
extern char scrolllock_mask;
extern char capslock_mask;

static int SIGTERM_SIGNAL = 0;

static char shortcutName[MAX_SHORTCUT_NUM][SHORTCUT_CONTENT_LEN] = { '\0' };
static char shortcutValue[MAX_SHORTCUT_NUM][SHORTCUT_CONTENT_LEN] = { '\0' };

Display* initXClient ()
{
    int screen;
    XInitThreads();
    XInitThreads();
    Display *display = XOpenDisplay ( NULL );
    if (!display)
    {   
        pbred ( "Open Display Failed" );
        exit (1);
    }   

    XAllowEvents (display, AsyncBoth, CurrentTime);

    for (screen = 0; screen < ScreenCount (display); screen++)
    {
        XSelectInput (display, RootWindow (display, screen),
                KeyPressMask | KeyReleaseMask | PointerMotionMask);
    }

    return display;
}

/* 为了使numlock、caplock、scrolllock锁定时仍能正常捕获对应按钮事件，有必要区分
 * 捕获, 对modifier进行或操作即可*/
    static void
my_grab_key (Display * dpy, KeyCode keycode, unsigned int modifier,
        Window win)
{
    modifier &= ~(numlock_mask | capslock_mask | scrolllock_mask);


    XGrabKey (dpy, keycode, modifier, (win ? win : DefaultRootWindow (dpy)),
            False, GrabModeAsync, GrabModeAsync);

    if (modifier == AnyModifier)
        return;

    if (numlock_mask)
        XGrabKey (dpy, keycode, modifier | numlock_mask,
                (win ? win : DefaultRootWindow (dpy)),
                False, GrabModeAsync, GrabModeAsync);

    if (capslock_mask)
        XGrabKey (dpy, keycode, modifier | capslock_mask,
                (win ? win : DefaultRootWindow (dpy)),
                False, GrabModeAsync, GrabModeAsync);

    if (scrolllock_mask)
        XGrabKey (dpy, keycode, modifier | scrolllock_mask,
                (win ? win : DefaultRootWindow (dpy)),
                False, GrabModeAsync, GrabModeAsync);

    if (numlock_mask && capslock_mask)
        XGrabKey (dpy, keycode, modifier | numlock_mask | capslock_mask,
                (win ? win : DefaultRootWindow (dpy)),
                False, GrabModeAsync, GrabModeAsync);

    if (numlock_mask && scrolllock_mask)
        XGrabKey (dpy, keycode, modifier | numlock_mask | scrolllock_mask,
                (win ? win : DefaultRootWindow (dpy)),
                False, GrabModeAsync, GrabModeAsync);

    if (capslock_mask && scrolllock_mask)
        XGrabKey (dpy, keycode, modifier | capslock_mask | scrolllock_mask,
                (win ? win : DefaultRootWindow (dpy)),
                False, GrabModeAsync, GrabModeAsync);

    if (numlock_mask && capslock_mask && scrolllock_mask)
        XGrabKey (dpy, keycode,
                modifier | numlock_mask | capslock_mask | scrolllock_mask,
                (win ? win : DefaultRootWindow (dpy)), False, GrabModeAsync,
                GrabModeAsync);

}

void initListenKeys ( Display *display, int *keys ) {
    int screen = 0;
    int i = 0;

    for ( ; screen < ScreenCount ( display ); screen++ ) {

        for ( i=0; i<MAX_SHORTCUT_NUM; i++ ) {

            if ( keys[i] ) {
                my_grab_key ( display, keys[BASE+i], keys[i], RootWindow(display, screen) ); 
                pred("grab: KeyCode:%d ModifierMask:%d", keys[BASE+i], keys[i]);
            }

            if ( ! keys[i] ) break;
        }
    }

    /* my_grab_key ( display, 133, 64,  win ); */ 
}

static int * null_X_error (Display * display, XErrorEvent * e)
{
    static int already = 0;

    /* The warning is displayed only once */
    if (already != 0) return (NULL);
    already = 1;

    printf ("\n*** Warning ***\n");
    printf ("Please verify that there is not another program running\n");

    return (NULL);
}

void exitNotify ( ) {
    SIGTERM_SIGNAL = 1;
}

char *getShortcutName ( char *shortcut ) {

    for ( int i=0; i<MAX_SHORTCUT_NUM; i++ ) {
        if ( ! *shortcutValue[i] ) return NULL;
        if ( strcmp ( shortcutValue[i], shortcut ) == 0 ) {
            pbgreen ( "Found match shortcut: %s", shortcutName[i] );
            return shortcutName[i];
        }
    }

    return NULL;
}

void shortcutReceiveNotify ( 
        char *modifiers,
        char *key,
        char *shmaddr_pic,
        char *shmaddr_keyboard,
        char *shmaddr_setting
        ) {

    char shortcutValue[128];
    char *shortcutName = NULL;
    strcpy ( shortcutValue, modifiers );
    strcat ( shortcutValue, key );

    shortcutName = getShortcutName ( upperCase(shortcutValue) );
    if ( !shortcutName )
        return;

    if ( strstr ( shortcutName, "Quick-Search" ) ) {
        if ( shmaddr_keyboard[WINDOW_OPENED_FLAG] == '1' ) return;

        pcyan ( "Quick-Search-Shortcut" );
        shmaddr_keyboard[QuickSearchShortcutPressed_FLAG] = '1';
        shmaddr_keyboard[QUICK_SEARCH_NOTIFY] = '1';

    } else if ( strstr ( shortcutName, "Translate-Picture" ) ) {
        pcyan ( "Translate-Picture-Shortcut" );
        shmaddr_pic[1] = '1';

    } else if ( strstr ( shortcutName, "Toggle-Monitoring" ) ) {
        pcyan ( "Toggle-Monitoring-Shortcut" );
        shmaddr_keyboard[SELECT_EXCLUDE_FLAG] = '1';
    } else if ( strstr ( shortcutName, "Open-Setting-Window" ) ) {
        pcyan ( "Open-Setting-Window-Shortcut" );
        if ( shmaddr_setting[1] == '0' )
            shmaddr_setting[0] = '1';
    }

    /* pcyan ( "Return " ); */
}


static void eventLoop (Display * display, int *listenKeys)
{
    XEvent e;
    char  *keystr;
    char keystrArray[16];
    char  modifierstr[16];

    int fd;
    /* int readfd; */
    fd_set fds;
    struct timeval tv;

    /* 多个程序同时grab一组键值会导致错误发生*/
    XSetErrorHandler ((XErrorHandler) null_X_error);

    fd = ConnectionNumber ( display );

    /* typedef struct {                 KeyPress or KeyRelease  */
    /* int type;                        # of last request processed by server  */
    /* unsigned long serial;            true if this came from a SendEvent request  */
    /* Bool send_event;                 Display the event was read from  */
    /* Display *display;                ‘‘ev ent’’ window it is reported relative to  */
    /* Window window;                   root window that the event occurred on  */
    /* Window root;                     child window  */
    /* Window subwindow;                milliseconds  */
    /* Time time;                       pointer x, y coordinates in event window  */
    /* int x, y;                        coordinates relative to root  */
    /* int x_root, y_root;              key or button mask  */
    /* unsigned int state;              detail  */
    /* unsigned int keycode;            same screen flag  */
    /* Bool same_screen; */
    /* } XKeyEvent;  */             

    struct sigaction sa;
    sa.sa_handler = exitNotify;
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGTERM, &sa, NULL) != 0 ) {
        pred ( "sigaction exec failed in shortcutListener.c. function: event_loop" );
        XCloseDisplay ( display );
        exit(1);
    }

    char *shmaddr_pic = NULL;
    char *shmaddr_keyboard = NULL;
    char *shmaddr_setting = NULL;
    shared_memory_for_pic ( &shmaddr_pic );
    shared_memory_for_keyboard_event ( &shmaddr_keyboard );
    shared_memory_for_setting ( &shmaddr_setting );
    if ( ! shmaddr_pic || ! shmaddr_keyboard || !shmaddr_setting) {
        pred ( "Got shared memory failed in shortcutListener!!!" );
        quit();
    }

    pbcyan ( "Get shared memory address successful" );
    memset ( shmaddr_pic, '\0', SHMSIZE );
    memset ( shmaddr_keyboard, '\0', 100 );

    readFromConfigByKeyword ( shortcutValue, "Shortcut" );
    readNameByKeyword ( shortcutName, "Shortcut" );

    struct pollfd pfd;
    int timeout = 1000;
    pfd.fd = fd;
    pfd.events = POLLIN|POLLPRI;

    while (True)
    {

        /* FD_SET ( fd, &fds ); */
        /* tv.tv_sec = 1; */
        /* tv.tv_usec = 0; */


        /* 主要用于延时但又可以保证有事件到来不会被继续阻塞*/
        /* select ( fd+1, &fds, NULL, NULL, &tv ); */
        poll ( &pfd, 1, timeout );
        memset ( &e, '\0', sizeof(e) );

        /* The XPending function returns the number of events that have been received 
         * from the X server but have not been removed from the event queue.
         * 没有event存在xpending返回0, 不会执行XnextEvent防止阻塞*/
        while(XPending(display))
            /* The XNextEvent function copies the first event from the event queue 
             * into the specified XEvent structure and then removes it from the queue.
             * If the event queue is empty, XNextEvent flushes the output buffer and
             * blocks until an event is received.
             *
             * 如上英文所述: 如果有event在队列，会被复制到XEvent结构体中,并将其从队列
             * 中移除，而上面Xpending会保证在
             * 有event到来才会执行下面的语句，所以可以保证程序不会被下面这条语句阻塞*/
            XNextEvent(display, &e);


        /* fprintf(stdout, "TEST POINT\n"); */

        switch (e.type)
        {
            case KeyPress: 
                /* printf("KeyPress: %d\n", ((XKeyEvent*)&e)->keycode); */

                /* Method 1. Cannot identify shift level*/
                /* keystr = XKeysymToString( */
                /*         XkbKeycodeToKeysym ( display, e.xkey.keycode, 0, e.xkey.state & ShiftMask ? 1 : 0 )); */

                /* Method 2. Cannot identify shift level*/
                keystr = XKeysymToString ( XLookupKeysym ( &e.xkey, 0 ) );
                strcpy ( keystrArray, keystr );
                /* pmag ( "Press: %s%s", mask2str(e.xkey.state, modifierstr ),keystrArray); */

                /* Method 3. Can identify shift level.但是有些情况下无法识别出key，如Shift+Control+J*/
                /* keysym = XLookupKeysym ( &e.xkey, 0 ); */
                /* XLookupString ( */ 
                /*         &e.xkey, keystrArray, sizeof(keystrArray), */
                /*         &keysym, */
                /*         NULL */
                /*         ); */

                /* pmag ( "Press: %s%s", mask2str(e.xkey.state, modifierstr ),keystrArray ); */

                shortcutReceiveNotify ( 
                        mask2str(e.xkey.state, modifierstr),
                        keystrArray,
                        shmaddr_pic,
                        shmaddr_keyboard,
                        shmaddr_setting
                        );

                break;

            case KeyRelease:
                /* printf("KeyRelease: %d\n", ((XKeyEvent*)&e)->keycode); */
                break;

            case ButtonPress:
                printf("Button Press\n");
                break;

            case ButtonRelease: 
                printf("Button Release\n");
                break;

            default: 
                /* pbcyan ( "Default: e.type=%d", e.type );; */
                break;
        }

        if ( SIGTERM_SIGNAL ) break;
    }

    pbcyan ( "Close XDisplay in shortcutListener" );
    XCloseDisplay ( display );
}

int listenShortcut()
{

    Display *display = NULL;

    static int *keys = NULL;
    display = initXClient( );
    getModifiersMapping ( display );
    keys = extractShortcut( display );
    initListenKeys ( display, keys );

    /* pblue ( "%d", XKeysymToKeycode( display, XStringToKeysym("v") ) ); */
    /* pblue ( "%d", XKeysymToKeycode( display, XK_1 ) ); */

    eventLoop ( display, keys );

    pbcyan ( "Shortcut Listener 退出" );

    return 0;
}
