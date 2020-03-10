#include "common.h"
#include <glib-object.h>
#include <gobject/gvaluecollector.h>
#include "audio.h"
#include "newWindow.h"
#include "windowData.h"
#include "configControl.h"
#include "useful.h"
#include "configControl.h"

/* 键值掩码在gdk/gdkkeysyms.h*/

extern char audioOnline_en[512];
extern char audioOnline_uk[512];
extern char audioOffline_en[512];
extern char audioOffline_uk[512];

gboolean on_key_press_cb ( 
        GtkWidget *window,
        GdkEventKey *event,
        gpointer *data ) {

    int mask = event->state;
    int keyval = event->keyval;
    int upperKeyval = gdk_keyval_to_upper ( keyval );

    gboolean enableCtrlCToClose = FALSE;
    ConfigData *cd = WINDATA(data)->cd;

    enableCtrlCToClose = cd->ctrlCToClose;

    pbblue ( "Key Press mask & keyval upperkeyval %d %d %d ",
            mask, keyval, upperKeyval );

    if ( event->state ==  GDK_CONTROL_MASK ) {
        if ( event->keyval == GDK_KEY_c && enableCtrlCToClose ) {

            g_print ("Captured Control+C, destroying window\n");
            destroyNormalWin ( window, (WinData*)data );
        }
    }

    if ( ((cd->switchSourceMask & mask) || cd->switchSourceMask == mask ) && 
            (cd->switchSourceKeyval == keyval || cd->switchSourceKeyval == upperKeyval) ) {

        changeDisplay(GET_BUTTON(data, WINDATA(data)->who), data);
    }

    if ( (cd->playAudioKeyval == keyval || cd->playAudioKeyval == upperKeyval  ) &&
            ((cd->playAudioKeyval & mask) || mask == cd->playAudioMask) ) {

        if ( (strlen(audioOnline_en)==0 && strlen(audioOnline_uk)==0) \
                && (strlen(audioOffline_en)==0 && strlen(audioOffline_uk)==0) ) {

            g_print ("No audio (keyPress.c)\n");
            return TRUE;
        }

        bw.audio_online[0] = audioOnline_en;
        bw.audio_online[1] = audioOnline_uk;

        mw.audio_offline[0] = audioOffline_en;
        mw.audio_offline[1] = audioOffline_uk;

        mp3play (NULL, data);
    }

    return TRUE;
}
