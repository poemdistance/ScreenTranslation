#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"

typedef struct {

    char **bing_result[BING_SIZE];
    char *google_result[GOOGLE_SIZE];
    char **mysql_result[MYSQL_SIZE];

    char *tmp;

    char *text;
    char *previousText;

}MemoryData ;

#endif
