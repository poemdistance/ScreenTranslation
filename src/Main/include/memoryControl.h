#ifndef __INIT_MEMORY__
#define __INIT_MEMORY__

#define ZH_EN_TRAN_SIZE 20
#define PER_SENTENCE_SIZE (1024*1024)

void initMemoryMysql ( char ***mysql_result );
void initMemoryBing ( char ***bing_result );
void initMemoryGoogle ( char **google_result );
void initMemoryTmp ( char **tmp );

void releaseMemoryGoogle ( char **google_result );
void releaseMemoryMysql  ( char ***mysql_result );
void releaseMemoryBing   ( char ***bing_result );
void releaseMemoryTmp    ( char *tmp );

void clearMemory ( void *data );
void clearBingMysqlResultMemory ( char ***bing_result, char ***mysql_result  );

#endif

