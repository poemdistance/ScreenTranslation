#ifndef __quickSearch_H__
#define __quickSearch_H__

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <linux/input.h>
#include <sys/ipc.h>

#include "sharedMemory.h"

#define TEXT_SUBMIT_FLAG ( 20 )
#define SUBMIT_TEXT ( 21 )

char **getKeyboardDevice(char (*buf)[100]);
void captureShortcutEvent(int socket);
void err_exit_qs(const char *buf);
void searchWindow();
void quickSearch();

char *itoa ( int num );

#endif
