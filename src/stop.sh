#!/bin/bash

#process=("mstran" "./stran" "stran" "tranen" "bdtran" "fetchDict" "extractPic")
process=("stran")

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

for n in $(seq $len)
do
    pid=`ps -aux | grep ${process[n-1]} | head -n 1 |awk '{print $2}'|xargs`
    if [ $pid -ne $$ ]; then
        if [ $pid -ne $startup ]; then
            kill -15 $pid #>/dev/null 2>&1
        fi
    fi
done

