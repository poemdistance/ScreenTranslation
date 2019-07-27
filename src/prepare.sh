#!/bin/bash

#复制文件到家目录
storage="$HOME/.stran"
currFile=("../gif_pic/background.jpg" "../gif_pic/tran.png")

declare -i len
len=${#currFile[*]}-1

mkdir $storage
echo
for i in $(seq 0 $len)
do
    cp ${currFile[i]} $storage -v
done

#修改文件路径
needChangFile=("./newWindow.c" "./GuiEntry.c")
len=${#needChangFile[@]}-1
src="\/home\/rease"
dst="\/"$HOME

for i in $(seq 0 $len)
do
    sed -i 's/$src/$dst/' ${needChangFile[i]}
done

#加入设备文件用户组
group=`ls -l /dev/input/mice | awk '{print $4}' | xargs`
echo
echo "/dev/input/mice is belong to the group of "\"$group\"
echo "Adding current user to group "\"$group\"
sudo usermod -aG $group $USER

#修改键盘设备文件
device=`cat /var/log/Xorg.0.log | grep keyboard | awk '{print $4}' | tail -n 1 |xargs`
device2=`cat /var/log/Xorg.1.log | grep keyboard | awk '{print $4}' | tail -n 1 |xargs`

echo
if [ -n "$device" ] 
then
    echo "found the actual keyboard device: /dev/input/"$device
    actualDevice=device
fi

if [ -n "$device2" ]
then
    echo "found the actual keyboard device: /dev/input/"$device2
    actualDevice=device2
fi

if [ -n "$actualDevice" ]
then
    src="\/dev\/input\/event3"
    dst="\/dev\/input\/$actualDevice"
    inFile="detectMouse.c"
    sed -i 's/$src/$dst/g' $inFile
else
    echo "Didn't find out the actual keyboard device"
    echo "Please figure it out by yourself and modify the detectMouse.c"
fi
