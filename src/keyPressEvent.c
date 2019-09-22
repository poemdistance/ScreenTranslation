#include "common.h"
#include <glib-object.h>
#include <gobject/gvaluecollector.h>

/* 键值掩码在gdk/gdkkeysyms.h*/

extern char audio_en[512];
extern char audio_uk[512];
extern int show;

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
        g_print ("Press Space\n");

        printf("\033[0;34maudio_en:%s \033[0m\n", audio_en);
        printf("\033[0;34maudio_uk:%s \033[0m\n", audio_uk);

        if ( (strlen(audio_en)==0 && strlen(audio_uk)==0)  || !show ) {
            g_print ("No audio\n");
            return TRUE;
        }

        bw.audio[0] = audio_en;
        bw.audio[1] = audio_uk;

        mp3play (NULL, data);
    }

    return TRUE;
}
