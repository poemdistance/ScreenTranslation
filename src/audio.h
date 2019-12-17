#ifndef __audio__
#define __audio__

#include "windowData.h"

int mp3play (GtkWidget *button, gpointer *data);

GtkWidget* newVolumeBtn () ;

GtkWidget* insertVolumeIcon\
        ( GtkWidget *window, GtkWidget *layout, WinData *wd, int type ) ;

void syncVolumeBtn ( WinData *wd, int type );

#endif
