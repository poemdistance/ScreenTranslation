#include "quickSearch.h"

void captureShortcutEvent(int socket) {

    int fd;
    char device[100];
    fd = open( getKeyboardDevice(device) , O_RDONLY);

    struct timeval tv;
    fd_set fdset;
    int retnum = -1;

    struct input_event ev;

    int AltPress = 0;
    int CtrlPress = 0;
    //int count = 0;

    char *shmaddr;
    shared_memory_for_keyboard_event(&shmaddr);

    //char buf[2] = { '\0' };

    for (;;) {

        tv.tv_usec = 200000;
        tv.tv_sec = 0;

        FD_ZERO ( &fdset );
        FD_SET ( fd, &fdset );

        retnum = select ( fd+1, &fdset, NULL, NULL, &tv );

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

        if ( read ( fd, &ev, sizeof(ev) ) < 0 )
            continue;

        /* 跳过无用的keycode*/
        if ( ev.code == 4 || ev.code == KEY_RESERVED )
            continue;

        if ( AltPress ) {

            if ( ev.code == KEY_J ) {

                fprintf(stdout, "Captured pressing event <Alt-J>\n");
                write ( socket, "1", 1 );
                shmaddr[0] = '1';
            }

            AltPress = 0;
        }

        if ( CtrlPress ) {

            if ( ev.code == KEY_C ) {

                fprintf(stdout, "Captured pressing event <Ctrl-C>\n");
                if ( shmaddr[2] == '1')
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
