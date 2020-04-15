#ifndef __STR_MASK__
#define __STR_MASK__

#include <X11/Xlib.h>

void getModifiersMapping (Display * dpy);
int str2mask( char *str );
char* mask2str( int mask, char *result );
char *toStr( int mask );
int unusedMask();
char *getKeyString ( char *str );
int *extractShortcut ( Display *display );
void *listenShortcut ( void *data );
char *getRawKeyString ( char *str );

#endif
