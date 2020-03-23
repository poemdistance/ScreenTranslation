#ifndef __SHORTCUT_LISTERNER__
#define __SHORTCUT_LISTERNER__

#include <X11/Xlib.h>

void getModifiersMapping (Display * dpy);
int str2mask( char *str );
char* mask2str( int mask, char *result );
char *toStr( int mask );
int *extractShortcut ( Display *display );
int listenShortcut();
char *getKeyString ( char *str );
char *getRawKeyString ( char *str );
int unusedMask();

#endif
