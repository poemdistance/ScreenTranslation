#!/bin/sh

for i in $(seq 10)
do
    kill -9 `ps -aux | grep mstran | head -n 1 |awk '{print $2}'|xargs` >/dev/null 2>&1
    kill -9 `ps -aux | grep ./stran | head -n 1 |awk '{print $2}'|xargs`>/dev/null 2>&1
    kill -9 `ps -aux | grep stran | head -n 1 |awk '{print $2}'|xargs`>/dev/null 2>&1
    kill -9 `ps -aux | grep tranen | head -n 1 |awk '{print $2}'|xargs`>/dev/null 2>&1
    kill -9 `ps -aux | grep bdtran | head -n 1 |awk '{print $2}'|xargs`>/dev/null 2>&1
done

