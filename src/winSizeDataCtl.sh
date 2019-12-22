#!/bin/bash

if [ $# -lt 3 ]; then
    echo 'Usage:  bash [path] num1 num2 file.data'
    exit
fi

val=$1
size=$2
file=$3

if [ ! -f $file ]; then
    echo $file" doesn't exist"
    exit
fi

output=`sed -n "/^$val.*/p" $file`
echo $output

if [ ! -n "$output" ]; then 
    echo $val' '$size >> $file
else
    sed -i "s/^$val.*/$val $size/g" $file
fi

cat $file |  sort -n -s -k1,1 -o $file
