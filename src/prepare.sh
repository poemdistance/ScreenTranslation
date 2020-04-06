#!/bin/bash


YELLOW='\033[0;33m'
GREEN='\033[0;34m'
END='\033[0m'

bash stop.sh
echo "Use server for pip: "$specific

sed -i "s/\$USER/$USER/g" Mstran.desktop

#复制文件到家目录
storage="$HOME/.stran"
currFile=("../gif_pic/background.jpg" "../gif_pic/tran.png" "../gif_pic/switch.png"\ 
    "./startup.sh" "./errNotification.sh" "../gif_pic/google.png" "stop.sh");

resourcesDir=("../ui/" "../config/")

declare -i len
len=${#currFile[*]}-1

echo 'Copying resources...'
mkdir $storage
mkdir $storage/pic
echo
for i in $(seq 0 $len)
do
    out=`cp ${currFile[i]} $storage -v`
    echo -e ${YELLOW}"Copying file: "${out}${END}
done

len=${#resourcesDir[*]}-1
for i in `seq 0 $len`
do
    echo
    echo -e ${GREEN}"Entering dir: "${resourcesDir[i]}${END}
    for f in `ls ${resourcesDir[i]} -a`
    do
        if [[ "$f" != "." && "$f" != ".." ]]; then
            out=`cp  "${resourcesDir[i]}""$f" $storage -v`
            echo -e ${YELLOW}"Copying file "$out${END}
        fi
    done
    echo
done

cd ../baidu-translate
git pull origin master
cd ../google-translate
git pull origin master
cd ../gnome-screenshot
git pull origin mainline

#git submodule foreach git pull #origin master
sudo pip3 install  mysql-connector-python $specific

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
sudo pip3 install -r requirements.txt $specific
sudo ./setup.py install --record files.txt
sudo chmod -c 775 /usr/bin/bdtran
cat files.txt |xargs sudo chmod -c 775

cd ../google-translate
sudo pip3 install -r requirements.txt $specific
sudo ./setup.py install --record files.txt
sudo pip3 install  mysql-connector-python $specific
sudo chmod -c 775 /usr/bin/tranen
cat files.txt |xargs sudo chmod -c 775

cd ../BingTran
sudo pip3 install -r requirements.txt $specific
sudo ./setup.py install --record files.txt
sudo chmod -c 775 /usr/bin/bing
cat files.txt |xargs sudo chmod -c 775

system=`uname -a`
if [[ $system =~ .*"deepin".* ]]; then
    sudo apt install gnome-screenshot
else
    cd ../gnome-screenshot
    sudo bash install.sh
fi
