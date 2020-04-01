#include "common.h"
#include <ctype.h>

int isEmpty(char *buf) {

    if ( ! buf ) return 1;

    char *p = buf;
    while ( *p && isspace(*p) ) p++;
    return *p == '\0';
}
