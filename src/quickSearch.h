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

#define TEXT_SUBMIT_BYTE ( 20 )
#define SUBMIT_TEXT ( 21 )

char **getKeyboardDevice(char (*buf)[100]);
void captureShortcutEvent(int socket);
void err_exit_qs(const char *buf);
void searchWindow();

char *itoa ( int num );

int shared_memory_for_quickSearch(char **addr);
int shared_memory_for_keyboard_event(char **addr);

#endif
