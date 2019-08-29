#!/bin/bash

#复制文件到家目录
storage="$HOME/.stran"
currFile=("../gif_pic/background.jpg" "../gif_pic/tran.png" "../gif_pic/Switch.png" 
    "../gif_pic/volume.png" "./startup.sh" "./errNotification.sh" )

declare -i len
len=${#currFile[*]}-1

echo
mkdir $storage
echo
for i in $(seq 0 $len)
do
    cp ${currFile[i]} $storage -v
done

#修改资源文件路径以适应当前用户
needChangFile=("./newWindow.c" "./GuiEntrance.c" "audioPlayer.c" "Mstran.desktop")
len=${#needChangFile[@]}-1
src="/home/rease"
dst=$HOME

echo

for i in $(seq 0 $len)
do
    
    sed -i "s~${src}~${dst}~g" ${needChangFile[i]}
    echo "Changing ${needChangFile[i]} to adapt the current user successful"

done

#复制启动图标，创建日志文件,修改读写权限
sudo cp Mstran.desktop /usr/share/applications/ -v
sudo touch /var/log/mstran.log
sudo chmod -c 750 /var/log/mstran.log
sudo chown -c $USER /var/log/mstran.log

#加入设备文件用户组
group=`ls -l /dev/input/mice | awk '{print $4}' | xargs`
echo
if [ -n "$group" ]
then    
    echo "Note: Please check whether the device 'mice' is belong to '$group'"
    echo "because it might be wrong result"
    echo
    echo "/dev/input/mice is belong to the group of "\"$group\"
    echo "Adding current user to group "\"$group\"
    sudo usermod -aG $group $USER

    echo
    echo "注意: 加入的组在重启后生效，在此之前需要使用sudo执行程序"
    echo
fi
