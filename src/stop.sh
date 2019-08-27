#!/bin/sh

kill `ps -aux | grep mstran | head -n 1 |awk '{print $2}'|xargs`
