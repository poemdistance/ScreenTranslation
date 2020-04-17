#ifndef __SHARED_MEMORY__
#define __SHARED_MEMORY__

#define GETEKYDIR ("/tmp")
#define PROJECTID  (2333)
#define PROJECTID2  (2334)
#define PIC_PROJECT  (2339)
#define SETTING_PROJECT ( 2340 )
#define SHMSIZE (1024*1024)


int shared_memory_for_google_translate(char **addr);
int shared_memory_for_bing_translate(char **addr);
int shared_memory_for_selection(char **addr);
int shared_memory_for_mysql(char **addr);
int shared_memory_for_pic(char **addr);
int shared_memory_for_quickSearch(char **addr);
int shared_memory_for_keyboard_event(char **addr);
int shared_memory_for_setting ( char **addr );

#endif

