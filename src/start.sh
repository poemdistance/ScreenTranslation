#!/bin/sh

kill `ps -aux | grep mstran | head -n 1 |awk '{print $2}'|xargs` > /dev/null 2>&1
(nohup /usr/bin/mstran >/dev/null 2>&1) &
notify-send 'ScreenTranslation' '取词翻译正在运行' --icon=dialog-information
