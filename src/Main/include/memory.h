#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"

typedef struct {

    char **bing_result[BING_SIZE];
    char *google_result[GOOGLESIZE];
    char **mysql_result[MYSQLSIZE];

    char *tmp;

    char *text;
    char *previousText;

}MemoryData ;

#endif
