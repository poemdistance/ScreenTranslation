#include "common.h"
#include <glib-object.h>
#include <gobject/gvaluecollector.h>
#include "audio.h"
#include "newWindow.h"

/* 键值掩码在gdk/gdkkeysyms.h*/

extern char audioOnline_en[512];
extern char audioOnline_uk[512];
extern char audioOffline_en[512];
extern char audioOffline_uk[512];

gboolean key_press ( GtkWidget *window, GdkEventKey *event, gpointer *data ) {

    if ( event->state ==  GDK_CONTROL_MASK ) {
        if ( event->keyval == GDK_KEY_c) {

            g_print ("Captured Control-C, destroying window\n");
            destroyNormalWin ( window, (WinData*)data );
        }
    }

    if ( event->keyval == GDK_KEY_Return ) {
        g_print ("Key Return\n");
        changeDisplay(NULL, data);
    }

    if ( event->keyval == GDK_KEY_space ) {
        g_print ("Press Space (keyPress.c)\n");

        if ( (strlen(audioOnline_en)==0 && strlen(audioOnline_uk)==0) \
                && (strlen(audioOffline_en)==0 && strlen(audioOffline_uk)==0) ) {

            g_print ("No audio (keyPress.c)\n");
            return TRUE;
        }

        bw.audio_online[0] = audioOnline_en;
        bw.audio_online[1] = audioOnline_uk;

        bw.audio_offline[0] = audioOffline_en;
        bw.audio_offline[1] = audioOffline_uk;

        mp3play (NULL, data);
    }

    return TRUE;
}
