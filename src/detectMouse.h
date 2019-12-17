#ifndef __detectMouse_h__
#define __detectMouse_h__

void show_utf8_prop(Display *dpy, Window w, Atom p, char *text);

int getClipboard(char *text);
void delay();
void writePipe(char *text, int fd);
void handler(int signo);
int isApp(char *appName, char *name);
int previous( int n );
int isAction(int history[], int last, int action);
void sync_key( int *fd, struct input_event *event, int *len);
void press(int fd, int keyCode);
void release(int fd, int keyCode);
void simulateKey(int fd,  int key[], int len);
void *GuiEntrance(void *arg);
void *DetectMouse(void *arg);
void notify(int (*history)[4], int *thirdClick, int *releaseButton, int fd[2]);

#endif
