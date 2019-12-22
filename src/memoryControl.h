#ifndef __INIT_MEMORY__
#define __INIT_MEMORY__

void initMemoryMysql();
void initMemoryBaidu();
void initMemoryGoogle();
void initMemoryTmp();

void releaseMemoryGoogle();
void releaseMemoryMysql();
void releaseMemoryTmp();

void clearMemory();

#endif

