/* 部分源码来自:
 * https://github.com/nibrahim/showkeys/blob/master/tests/record-example.c*/

#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <X11/Xlib.h>
#include <X11/keysym.h> /* KeySym*/
#include <X11/Xutil.h> /* XLookupString*/
#include <X11/X.h> /* Some mask (ContrlMask...)*/
#include <X11/XKBlib.h>
#include <X11/Xlibint.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/extensions/record.h>
#include <X11/extensions/XTest.h>
#include <assert.h>
#include <sys/signal.h>
#include <string.h>
#include "printWithColor.h"
#include "strmask.h"
#include "configControl.h"
#include "sharedMemory.h"
#include "common.h"
#include "useful.h"
#include "cleanup.h"
#include "expanduser.h"
#include "windowData.h"

extern char modifier[7][10];
extern int modifier2maskTable[7];

extern int numlock_mask;
extern int scrolllock_mask;
extern int capslock_mask;

static char shortcutName[MAX_SHORTCUT_NUM][SHORTCUT_CONTENT_LEN] = { '\0' };
static char shortcutValue[MAX_SHORTCUT_NUM][SHORTCUT_CONTENT_LEN] = { '\0' };

static char *shmaddr_pic = NULL;
static char *shmaddr_keyboard = NULL;
static char *shmaddr_setting = NULL;
static XRecordContext  rc;
static XRecordRange  *rr;
static CommunicationData *md = NULL;


/* for this struct, refer to libxnee */
typedef union {
    unsigned char    type ;
    xEvent           event ;
    xResourceReq     req   ;
    xGenericReply    reply ;
    xError           error ;
    xConnSetupPrefix setup;
} XRecordDatum;

/*
 * FIXME: We need define a private struct for callback function,
 * to store cur_x, cur_y, data_display, ctrl_display etc.
 */
static Display *data_display = NULL;
static Display *ctrl_display = NULL;

char *getShortcutName ( char *shortcut ) {

    for ( int i=0; i<MAX_SHORTCUT_NUM; i++ ) {
        if ( ! *shortcutValue[i] ) return NULL;
        if ( strcmp ( shortcutValue[i], shortcut ) == 0 ) {
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
        if ( shmaddr_keyboard[WINDOW_OPENED_FLAG] == '1' ) {
            pbred ( "Window has opened, return" );
            return;
        }

        pcyan ( "Quick-Search-Shortcut" );
        shmaddr_keyboard[QUICK_SEARCH_FLAG] = '1';
        shmaddr_keyboard[QUICK_SEARCH_NOTIFY] = '1';

    } else if ( strstr ( shortcutName, "Translate-Picture" ) ) {
        pcyan ( "Translate-Picture-Shortcut" );
        shmaddr_pic[1] = '1';

    } else if ( strstr ( shortcutName, "Toggle-Monitoring" ) ) {
        pcyan ( "Toggle-Monitoring-Shortcut" );
        shmaddr_keyboard[SELECT_EXCLUDE_FLAG] = '1';
    } else if ( strstr ( shortcutName, "Open-Setting-Window" ) ) {
        pcyan ( "Open-Setting-Window-Shortcut" );
        /* if ( shmaddr_setting[1] == '0' ) */
        shmaddr_setting[0] = '1';
    } else if ( strstr ( shortcutName, "Recall-Previous-Translation" ) ) {
        shmaddr_keyboard[RECALL_PREVIOUS_TRAN] = '1';
        pcyan ( "Recall-Previous-Translation" );
    }

}

static void cleanup() {

    XRecordProcessReplies (data_display);

    XRecordDisableContext (ctrl_display, rc);
    XRecordFreeContext (ctrl_display, rc);
    XFree (rr);

    XCloseDisplay (data_display);
    XCloseDisplay (ctrl_display);

    pgreen ( "Close Display in shortcutListener" );
}

void event_callback(XPointer priv, XRecordInterceptData *hook )
{
    /* FIXME: we need use XQueryPointer to get the first location */
    static int cur_x = 0;
    static int cur_y = 0;
    char modifierstr[64];
    char *keystr = NULL;
    char keystrArray[64] = { '\0' };

    if (hook->category != XRecordFromServer) {
        XRecordFreeData (hook);
        return;
    }

    XRecordDatum *data = (XRecordDatum*) hook->data;

    struct stat st;
    static time_t lastModify = 0;
    const char *file = expanduser("/home/$USER/.stran/.configrc");

    stat ( file, &st );

    if ( st.st_mtime - lastModify > 0 ) {
        pbmag ( "更新监听快捷键" );
        lastModify = st.st_mtime;
        readFromConfigByKeyword ( shortcutValue, "Shortcut" );
        readNameByKeyword ( shortcutName, "Shortcut" );
        /* extractShortcut( ctrl_display ); */
    }

    /* int event_type = data->type; */
    int event_type = data->event.u.u.type;

    /* 如果是鼠标事件，则detail等于1,2,3等鼠标按钮*/
    int detail = data->event.u.u.detail;

    BYTE keycode;

    /* 相关定义可以在/usr/include/X11/Xproto.h的
     * typedef struct _xEvent找到*/
    keycode = data->event.u.u.detail;
    int mask = data->event.u.keyButtonPointer.state;

    int rootx = data->event.u.keyButtonPointer.rootX;
    int rooty = data->event.u.keyButtonPointer.rootY;
    int motionState = data->event.u.keyButtonPointer.state;
    /* int time = hook->server_time; */

    switch (event_type) {

        case KeyPress:

            /* Note: you should not use data_disp to do normal X operations !!!*/

            keystr = XKeysymToString(XKeycodeToKeysym(ctrl_display, keycode, 0));
            strcpy ( keystrArray, keystr );

            mask &= ~ (numlock_mask | scrolllock_mask | capslock_mask );

            shortcutReceiveNotify ( 
                    mask2str(mask, modifierstr),
                    keystr,
                    shmaddr_pic,
                    shmaddr_keyboard,
                    shmaddr_setting
                    );

            /* pblue ( "Keypress:%s%s", modifierstr, keystrArray ); */
            break;

        case KeyRelease: break;
        case ButtonPress:
                         if ( detail == 1 ){ md->buttonPress = 1; }
                         break;
        case ButtonRelease:
                         if ( detail == 1 ) { md->buttonRelease=1; }
                         break;
        case MotionNotify:
                         if ( md ) {
                             md->pointerx = cur_x = rootx;
                             md->pointery = cur_y = rooty;
                         } 
                         if ( motionState & Button1Mask ) {
                             if ( md && !(md->startSlide) ) {
                                 printf("Start Slide\n");
                                 md->startSlide = 1;
                             }
                         }
                         break;
        case CreateNotify: break;
        case DestroyNotify: break;
        case NoExpose: break;
        case Expose: break;
        default: break;
    }

    XRecordFreeData (hook);

    if ( md->sigtermNotify )
        XRecordDisableContext (ctrl_display, rc);
}

void initSharedMemory ( Arg *arg ) {

    shmaddr_pic = NULL;
    shmaddr_keyboard = NULL;
    shmaddr_setting = NULL;
    shared_memory_for_pic ( &shmaddr_pic );
    shared_memory_for_keyboard_event ( &shmaddr_keyboard );
    shared_memory_for_setting ( &shmaddr_setting );
    if ( ! shmaddr_pic || ! shmaddr_keyboard || !shmaddr_setting) {
        pred ( "Got shared memory failed in shortcutListener!!!" );
        quit ( arg );
    }

    pbcyan ( "Get shared memory address successful" );
    memset ( shmaddr_pic, '\0', SHMSIZE );
    memset ( shmaddr_keyboard, '\0', 100 );
}


void *listenShortcut ( void *data )
{
    Arg *arg = (Arg*)data;
    md = arg->md;

    md->startSlide = 0;
    md->buttonPress = 0;
    md->buttonRelease = 0;

    XInitThreads();
    ctrl_display = XOpenDisplay (NULL);
    data_display = XOpenDisplay (NULL);

    if (!ctrl_display || !data_display) {
        fprintf (stderr, "Error to open local display!\n");
        exit (1);
    }

    /* 
     * we must set the ctrl_display to sync mode, or, when we the enalbe 
     * context in data_display, there will be a fatal X error !!!
     */
    XSynchronize(ctrl_display,True);

    XRecordClientSpec  rcs;
    /* XRecordContext   rc; */

    rr = XRecordAllocRange ();
    if (!rr) {
        fprintf (stderr, "Could not alloc record range object!\n");
        exit (3);
    }

    rr->device_events.first = KeyPress;
    rr->device_events.last = MotionNotify;
    rcs = XRecordAllClients;

    rc = XRecordCreateContext (ctrl_display, 0, &rcs, 1, &rr, 1);
    if (!rc) {
        fprintf (stderr, "Could not create a record context!\n");
        exit (4);
    }


    initSharedMemory ( arg );
    readFromConfigByKeyword ( shortcutValue, "Shortcut" );
    readNameByKeyword ( shortcutName, "Shortcut" );

    pbgreen ( "获取Modifier映射" );
    getModifiersMapping ( ctrl_display );

    if (!XRecordEnableContext (data_display, rc, event_callback, NULL)) {
        fprintf (stderr, "Cound not enable the record context!\n");
        exit (5);
    }

    cleanup();

    return 0;
}
