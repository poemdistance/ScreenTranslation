#include "quickSearch.h"
#include "common.h"
#include <signal.h>

#define KEYBOARD_NUM ( 10 )

int fds[KEYBOARD_NUM] = { 0 };

void closeDevice() {

    printf("\033[0;31mCLOSING DEVICE (captureShortcutEvent)\033[0m\n");
    for ( int i=0; i<KEYBOARD_NUM; i++ ) {
        if ( fds[i] != 0 && fds[i] != 0 )
            close(fds[i]);
    }
    exit(0);
}

int *myopen( char (*dev)[100] )  {

    for ( int i=0; i<KEYBOARD_NUM; i++ ) {
        if ( strlen(dev[i]) > 0 ) {
            fds[i] = open((char*)dev[i], O_RDONLY);
            printf("\n\033[0;35mopen device %s <In captureShortcutEvent.c>\033[0m\n\n", dev[i]);
        }
    }

    return fds;
}

void captureShortcutEvent(int socket) {

    char device[KEYBOARD_NUM][100] = { '\0' };
    myopen( (char (*)[100])getKeyboardDevice(device) );
    int fd = 0;

    struct sigaction sa;
    sa.sa_handler = closeDevice;
    sigemptyset ( &sa.sa_mask );
    if ( sigaction ( SIGTERM, &sa, NULL) != 0 )
        err_exit_qs("Sigaction error in captureShortcutEvent");

    struct timeval tv;
    fd_set fdset;
    int retnum = 0;

    struct input_event ev;

    int AltPress = 0;
    int CtrlPress = 0;

    /* Just connect to the shared memory already exist */
    char *shmaddr;

    /* Byte 0: quick search 快捷键标志位(alt-j) <for newWindow.c>
     * Byte 1: 退出窗口快捷键标志位(ctrl-c) <for newWindow.c, 目前被屏蔽了>
     * Byte 2: 翻译窗口打开标志位
     * Byte 3: Alt-J 搜索窗口快捷键标志位(好像跟第0字节重复了，太久，啊啦也忘了)
     * Byte 4: 搜索窗口存在标志位
     * */
    shared_memory_for_keyboard_event(&shmaddr);

    char *shmaddr_pic = NULL;
    shared_memory_for_pic ( &shmaddr_pic );

    int maxfd = 0;

    for (;;) {

        tv.tv_usec = 300000;
        tv.tv_sec = 0;

        FD_ZERO ( &fdset );

        for ( int i=0; i<KEYBOARD_NUM; i++ ) {

            if ( fds[i] != 0 ) {

                FD_SET ( fds[i], &fdset );
                if ( fds[i] > maxfd )
                    maxfd = fds[i];
            }
        }

        retnum = select ( maxfd+1, &fdset, NULL, NULL, &tv );

        /* 超时*/
        if ( retnum == 0 )
            continue;

        /* value=0: release
         * value=1: press
         * value=2: auto repeat
         *
         * type: EV_KEY //What's this? 
         *  EV_KEY: - Used to describe state changes of keyboards, 
         *  buttons, or other key-like devices.
         *
         * code: KEY_*
         * */

        for ( int i=0; i<KEYBOARD_NUM; i++ )
            if ( fds[i] != 0 )
                if ( FD_ISSET ( fds[i], &fdset ) )
                    fd = fds[i];

        if ( read ( fd, &ev, sizeof(ev) ) < 0 )
            continue;

        /* 跳过无用的keycode*/
        if ( ev.code == 4 || ev.code == KEY_RESERVED )
            continue;

        /* Alt被按下，且搜索窗口和翻译结果展示窗口都未被打开*/
        if ( AltPress && shmaddr[4] != '1'  && shmaddr[2] != '1' ) {

            if ( ev.code == KEY_J ) {

                fprintf(stdout, "Captured pressing event <Alt-J>\n");
                //write ( socket, "1", 1 );
                shmaddr[3] = '1';

                /* quick search 快捷键标志位*/
                shmaddr[0] = '1';
            }

            if ( ev.code == KEY_D ) {

                fprintf(stdout, "Captured pressing event <Alt-D>\n");
                shmaddr_pic[1] = SCREEN_SHOT;
            }

            AltPress = 0;
        }

        if ( CtrlPress ) {

            if ( ev.code == KEY_C ) {

                fprintf(stdout, "Captured pressing event <Ctrl-C>\n");
                if ( shmaddr[2] == '1')
                    /* 退出快捷键标志位*/
                    shmaddr[1] = '1';
            }

            CtrlPress = 0;
        }

        if ( ! AltPress && ( ev.code == KEY_RIGHTALT || ev.code == KEY_LEFTALT ))
            AltPress = 1;

        if ( !CtrlPress && ( ev.code == KEY_LEFTCTRL || ev.code == KEY_RIGHTCTRL) )
            CtrlPress = 1;
    }
}
