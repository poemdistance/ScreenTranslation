#!/bin/bash

#执行格式: bash buttonPostionDataCtl.sh [num1] [num2] [num3] filename.data

if [ $# -lt 4 ]; then
    echo 'Not enough arguments'
    exit
fi

phoneticLen=$1
xpostion=$2
ypostion=$3
file=$4

if [ ! -f $file ]; then
    echo $file" doesn't exist"
    exit
fi

output=`sed -n "/^$phoneticLen.*/p" $file`
echo $output

if [ ! -n "$output" ]; then 
    echo $phoneticLen' '$xpostion' '$ypostion >> $file
else
    sed -i "s/^$phoneticLen.*/$phoneticLen $xpostion $ypostion/g" $file
fi

cat $file |  sort -n -s -k1,1 -o $file
