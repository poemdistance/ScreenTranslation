#ifndef __PRINT_WITH_COLOR__
#define __PRINT_WITH_COLOR__

#include <stdio.h>

#define pred(format, ...)  printf("\033[0;31m"format"\033[0m\n", ##__VA_ARGS__) //print with red color;
#define pgreen(format, ...)  printf("\033[0;32m"format"\033[0m\n", ##__VA_ARGS__)
#define pyellow(format, ...)  printf("\033[0;33m"format"\033[0m\n", ##__VA_ARGS__)
#define pblue(format, ...) printf("\033[0;34m"format"\033[0m\n", ##__VA_ARGS__)
#define pmag(format, ...) printf("\033[0;35m"format"\033[0m\n", ##__VA_ARGS__)
#define pcyan(format, ...) printf("\033[0;36m"format"\033[0m\n", ##__VA_ARGS__)

#define pbred(format, ...)  printf("\033[1;31m"format"\033[0m\n", ##__VA_ARGS__) //print with bold red color
#define pbgreen(format, ...)  printf("\033[1;32m"format"\033[0m\n", ##__VA_ARGS__)
#define pbyellow(format, ...)  printf("\033[1;33m"format"\033[0m\n", ##__VA_ARGS__)
#define pbblue(format, ...) printf("\033[1;34m"format"\033[0m\n", ##__VA_ARGS__)
#define pbmag(format, ...) printf("\033[1;35m"format"\033[0m\n", ##__VA_ARGS__)
#define pbcyan(format, ...) printf("\033[1;36m"format"\033[0m\n", ##__VA_ARGS__)


#endif
