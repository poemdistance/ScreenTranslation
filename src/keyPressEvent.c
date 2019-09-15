#include "common.h"
#include <glib-object.h>
#include <gobject/gvaluecollector.h>

gboolean key_press ( GtkWidget *window, GdkEventKey *event, gpointer *data ) {

    if ( event->state ==  GDK_CONTROL_MASK ) {
        if ( event->keyval == GDK_KEY_c) {

            g_print ("Captured Control-C, destroying window\n");
            destroyNormalWin ( window, (WinData*)data );
        }
    }

    return TRUE;
}
