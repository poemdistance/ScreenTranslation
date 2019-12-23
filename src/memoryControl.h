#ifndef __INIT_MEMORY__
#define __INIT_MEMORY__

#define ZH_EN_TRAN_SIZE 20
#define PER_SENTENCE_SIZE (1024*1024)

void initMemoryMysql();
void initMemoryBaidu();
void initMemoryGoogle();
void initMemoryTmp();

void releaseMemoryGoogle();
void releaseMemoryMysql();
void releaseMemoryBaidu();
void releaseMemoryTmp();

void clearMemory();
void clearBaiduMysqlResultMemory();

#endif

