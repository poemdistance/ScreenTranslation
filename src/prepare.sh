#!/bin/bash

bash stop.sh

#复制文件到家目录
storage="$HOME/.stran"
currFile=("../gif_pic/background.jpg" "../gif_pic/tran.png" "../gif_pic/switch.png"\ 
    "../gif_pic/volume.png" "./startup.sh" "./errNotification.sh" "../gif_pic/offline.png"\
    "../gif_pic/baidu.png" "../gif_pic/google.png" "../gif_pic/indicate.png" "stop.sh");

declare -i len
len=${#currFile[*]}-1

echo 'Copying resources...'
mkdir $storage
echo
for i in $(seq 0 $len)
do
    cp ${currFile[i]} $storage -v
done

git submodule foreach git pull origin master
sudo pip3 install  mysql-connector-python

#修改资源文件路径以适应当前用户
needChangFile=("background.c" "GuiEntrance.c" "audioPlayer.c" "Mstran.desktop" "switchButton.c" "debug.c")
len=${#needChangFile[@]}-1
src="/home/rease"
dst=$HOME

echo

for i in $(seq 0 $len)
do

    sed -i "s~${src}~${dst}~g" ${needChangFile[i]}
    echo "Changing ${needChangFile[i]} to adapt the current user successful"

done

echo 'Modify the connect user of mariadb'
sed -i "s/rease/$USER/g" ./fetchDict

#复制启动图标，创建日志文件,修改读写权限
sudo cp Mstran.desktop /usr/share/applications/ -v
sudo touch /var/log/mstran.log
sudo chmod -c 770 /var/log/mstran.log
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

echo
echo 'Preparing to install submodules'
echo

cd ../baidu-translate
sudo pip3 install -r requirements.txt
sudo ./setup.py install

cd ../google-translate
sudo pip3 install -r requirements.txt
sudo ./setup.py install
sudo pip3 install  mysql-connector-python

system=`uname -a`
if [[ $system =~ .*"deepin".* ]]; then
    sudo apt install gnome-screenshot
else
    cd ../gnome-screenshot
    sudo bash install.sh
fi
