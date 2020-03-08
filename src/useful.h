#ifndef __USERFUL__
#define __USERFUL__


int str2bool ( char *str );
char *bool2str ( int value );
int str2int ( char *str );
float str2float ( char *str, int digits );
double str2double ( char *str, int digits );

char *int2str ( int num, char *str );
char *float2str ( float num, char *str );
char *double2str ( double num, char *str );

char *upperCase ( char *str );
char *lowerCase ( char *str );

#endif
