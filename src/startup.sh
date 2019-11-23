#!/bin/sh

bash stop.sh
(nohup /usr/bin/mstran >>/var/log/mstran.log 2>&1) &
notify-send 'ScreenTranslation' '取词翻译正在运行' --icon=dialog-information
