#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "printWithColor.h"
#include <math.h>
#include "useful.h"

#define MAX_DOUBLE_DIGITS (15)
#define MAX_FLOAT_DIGITS  (7)

char *upperCase ( char *str ) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsequence-point"

    char *p = str;
    while ( p && *p ) {
        *p = toupper(*p);
        p++;
    }

    return str;
#pragma GCC diagnostic pop
}

char *lowerCase ( char *str ) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsequence-point"

    char *p = str;
    while ( p && *p ) {
        *p = tolower(*p);
        p++;
    }

    return str;
#pragma GCC diagnostic pop
}


double str2double ( char *str, int digits ) {

    if ( digits > MAX_DOUBLE_DIGITS ) {
        pbred ( "Double values can only support 15 digits!" );
        digits = 15;
    }

    /* Null pointer*/
    if ( !str ) return 0;

    char *p = str;
    while ( !isdigit(*p) && *p++);
    if ( p != str && ! *(p-1) )  return 0; /* No digit found*/


    double num = 0;
    double tmp = 0;
    int    foundDot = 0;
    int digitLen = 0;
    p = str;
    while ( *p ) {

        num = num*10 + *p++ - '0';

        if ( foundDot )
            if ( ++digitLen >= digits)
                break;

        if ( *p == '.' && ! foundDot) {
            foundDot = 1;
            tmp = num; /* Store the integer part*/
            num = 0;   /* Switch to the caculation of decimal part*/
            p++; /* Skip '.'*/
        }

        if ( ! isdigit ( *p ) ) break;
    }

    num = tmp + num * pow(10, -digitLen);

    return num;
}

float str2float ( char *str, int digits ) {

    if ( digits > MAX_FLOAT_DIGITS ) {
        pbred ( "Float values can only support 7 digits!" );
        digits = 7;
    }

    return (float)str2double ( str, digits );
}

int str2int ( char *str ) {

    if ( !str ) return 0;

    return atoi(str);
}

char *double2str ( double num, char *str ) {

    sprintf(str, "%lf", num);
    return str;
}

char *float2str ( float num, char *str ) {

    sprintf(str, "%f", num);
    return str;
}

char *int2str ( int num, char *str ) {
    sprintf(str, "%d", num);
    return str;
}

int str2bool ( char *str ) {

    if ( !str ) return 0;

    char buf[128];
    strcpy ( buf, str );
    upperCase ( buf );

    if ( strcmp ( buf, "TRUE" ) == 0 ) return 1;

    if ( strcmp ( buf, "FALSE" ) == 0 ) return 0;

    return 0;
}

char *bool2str ( int value ) {

    if ( value == 0 )
        return "FALSE";

    return "TRUE";
}


/* int main(int argc, char **argv) */
/* { */
/*     char str[100] = "-3"; */
/*     char *p = NULL; */
/*     printf("%d\n", str2int ( str )); */
/* } */
