#ifndef __FITTING__
#define __FITTING__

#define FOR_AUDIO_BUTTON 1
#define FOR_WIN_WIDTH 2
#define FOR_WIN_HEIGHT 3

#define DISABLE (0)
#define CLOSE (0)
#define OPEN (1)
#define FITTING_STATUS CLOSE

int getFitFunc(char *path, int forWhich, double *a, double *b, double *c, double *d, int disable);
void genFitFunc ( char *name, int disable );
int notExist ( char *path ) ;

#endif
