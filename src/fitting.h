#ifndef __FITTING__
#define __FITTING__

#define FOR_AUDIO_BUTTON 1
#define FOR_WIN_WIDTH 2
#define FOR_WIN_HEIGHT 3

int getFitFunc(char *path, int forWhich, double *a, double *b, double *c, double *d);
void genFitFunc ( char *name );
int notExist ( char *path ) ;

#endif
