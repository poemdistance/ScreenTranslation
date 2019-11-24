#!/bin/bash

bash /home/$USER/.stran/stop.sh $$
(nohup /usr/bin/stran >>/var/log/mstran.log 2>&1) &
notify-send 'ScreenTranslation' '取词翻译正在运行' --icon=dialog-information
