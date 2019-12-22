#!/bin/bash

bash stop.sh

#复制文件到家目录
storage="$HOME/.stran"
currFile=("../gif_pic/background.jpg" "../gif_pic/tran.png" "../gif_pic/switch.png"\ 
    "../gif_pic/audio.png" "./startup.sh" "./errNotification.sh" "../gif_pic/offline.png"\
    "../gif_pic/baidu.png" "../gif_pic/google.png" "../gif_pic/indicate.png" "stop.sh"\
    "../gif_pic/calibration.jpg" "../data/audioButtonPosition.data" "buttonPositionDataCtl.sh"\
    "winSizeDataCtl.sh");

declare -i len
len=${#currFile[*]}-1

echo 'Copying resources...'
mkdir $storage
mkdir $storage/pic
echo
for i in $(seq 0 $len)
do
    cp ${currFile[i]} $storage -v
done

chmod +x ~/.stran/winSizeDataCtl.sh

git submodule foreach git pull origin master
sudo pip3 install  mysql-connector-python

sed -i "s/\$USER/$USER/g" Mstran.desktop

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

cd ../BingTran
sudo pip3 install -r requirements.txt
sudo ./setup.py install

system=`uname -a`
if [[ $system =~ .*"deepin".* ]]; then
    sudo apt install gnome-screenshot
else
    cd ../gnome-screenshot
    sudo bash install.sh
fi
