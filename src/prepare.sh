#!/bin/bash


YELLOW='\033[0;33m'
GREEN='\033[0;34m'
END='\033[0m'

# bash stop.sh
echo "Use server for pip: "$specific

sed -i "s/rease/\$USER/g" Mstran.desktop
sed -i "s/\$USER/$USER/g" Mstran.desktop

#复制文件到家目录
storage="$HOME/.stran"
currFile=("../gif_pic/background.jpg" "../gif_pic/tran.png" "../gif_pic/switch.png"\ 
    "./startup.sh" "./errNotification.sh" "../gif_pic/google.png" "stop.sh");

resourcesDir=("../ui/")

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

echo
python3 ./mergeConfig.py

cd ../baidu-translate
git pull origin master
cd ../google-translate
git pull origin master

#git submodule foreach git pull #origin master
sudo pip3 install  mysql-connector-python $specific

cd ../src
#复制启动图标，创建日志文件,修改读写权限
sudo cp Mstran.desktop /usr/share/applications/ -v
sudo touch /var/log/mstran.log
sudo chmod -c 770 /var/log/mstran.log
sudo chown -c $USER /var/log/mstran.log

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

system=`awk -F= '$1=="ID" { print $2 ;}' /etc/os-release`
if [[ $system == deepin || $system == debian || $system == ubuntu ]]; then
    sudo apt install gnome-screenshot
elif [[ $system == arch || $system == manjaro ]]; then
    sudo pacman -S gnome-screenshot
else
    echo '未识别的发行版，请自行安装gnome-screenshot'
    echo '(目前可以不用安装，因截图识别已暂时被禁用)'
fi
