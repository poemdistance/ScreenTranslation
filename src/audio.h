#ifndef __audio__
#define __audio__

#include "windowData.h"

#define AUDIO_BASE_NAME "audioButtonPosition"
#define WIN_SIZE_BASE_NAME "winSizeInfo"

int mp3play (GtkWidget *button, gpointer *data);

GtkWidget* newAudioBtn () ;

GtkWidget* insertAudioIcon\
        ( GtkWidget *window, GtkWidget *layout, WinData *wd, int type ) ;

void syncAudioBtn ( WinData *wd, int type );
int getButtonPosition ( int x );

#endif
