#include "common.h"
#include <glib-object.h>
#include <gobject/gvaluecollector.h>
#include "audio.h"
#include "newWindow.h"
#include "windowData.h"
#include "configControl.h"
#include "useful.h"
#include "configControl.h"
#include "shortcutListener.h"

/* 键值掩码在gdk/gdkkeysyms.h*/

extern char audioOnline_en[512];
extern char audioOnline_am[512];
extern char audioOffline_en[512];
extern char audioOffline_am[512];

gboolean on_key_press_cb ( 
        GtkWidget *window,
        GdkEventKey *event,
        WinData *wd ) {

    int mask = event->state; 
    int keyval = event->keyval;
    int upperKeyval = gdk_keyval_to_upper ( keyval );

    mask &= ~ ( unusedMask() );

    gboolean enableCtrlCToClose = FALSE;
    ConfigData *cd = wd->cd;

    enableCtrlCToClose = cd->ctrlCToClose;

    pbblue ( "Key Press mask & keyval upperkeyval %d %d %d ",
            mask, keyval, upperKeyval );

    if ( mask ==  GDK_CONTROL_MASK ) {
        if ( upperKeyval == GDK_KEY_C && enableCtrlCToClose ) {

            g_print ("Captured Control+C, destroying window\n");
            destroyNormalWin ( window, wd );
        }
    }

    if ( ((cd->switchSourceMask & mask) || cd->switchSourceMask == mask ) && 
            (cd->switchSourceKeyval == keyval || cd->switchSourceKeyval == upperKeyval) ) {

        changeDisplay(GET_BUTTON(wd, wd->who), wd);
    }

    if ( (cd->playAudioKeyval == keyval || cd->playAudioKeyval == upperKeyval  ) &&
            ((cd->playAudioKeyval & mask) || mask == cd->playAudioMask) ) {

        if ( (strlen(audioOnline_en)==0 && strlen(audioOnline_am)==0) \
                && (strlen(audioOffline_en)==0 && strlen(audioOffline_am)==0) ) {

            g_print ("No audio (keyPress.c)\n");
            return TRUE;
        }

        mp3play (NULL, wd);
    }

    return TRUE;
}
