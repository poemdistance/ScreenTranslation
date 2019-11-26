#!/bin/bash

#process=("mstran" "./stran" "stran" "tranen" "bdtran" "fetchDict" "extractPic")
process=("stran" "fetchDict" "extractPic")

declare -i len
len=${#process[*]}

declare -i startup
echo "启动脚本pid:"$1
echo "参数长度:"$#
startup=0
if [ $# -gt 0 ]; then
    echo "参数长度大于0"
    startup=$1
fi

echo "startup:"$startup

for i in $(seq 10)
do
    for n in $(seq $len)
    do
        pid=`ps -aux | grep ${process[n-1]} | head -n 1 |awk '{print $2}'|xargs`
        if [ $pid -ne $$ ]; then
            if [ $pid -ne $startup ]; then
                kill -15 $pid >/dev/null 2>&1
                clear="Yes"
            fi
        fi
    done
done

if [[ $clear -ne "Yes" ]]; then
    echo "未发现翻译进程"
fi

