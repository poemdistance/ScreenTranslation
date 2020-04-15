CC = gcc
specific ?= 

all:
	+$(MAKE) -C CommonSrc
	+$(MAKE) -C Main
	+$(MAKE) -C Settings
	+$(MAKE) -C Monitor

install:
	# bash ./stop.sh
	sudo cp -v Main/bin/stran /usr/bin/
	sudo cp -v Monitor/bin/mstran /usr/bin/
	sudo cp -v Bash/stop.sh /usr/bin/stoptran
	sudo cp -v Python/fetchDict /usr/bin/
	sudo cp -v Python/extractPic /usr/bin
	sudo cp -v lib/libui.so /usr/lib
	sudo cp -v lib/libshared.so /usr/lib
	sudo cp -v DesktopFile/Mstran.desktop /usr/share/applications/
	sudo desktop-file-install /usr/share/applications/Mstran.desktop
	sudo update-desktop-database
	sudo chmod -c 775 /usr/bin/fetchDict
	sudo chmod -c 775 /usr/bin/extractPic
	sudo chmod -c 775 /usr/bin/mstran
	sudo chmod -c 775 /usr/bin/stran

clean:
	+$(MAKE) -C CommonSrc clean
	+$(MAKE) -C Main clean
	+$(MAKE) -C Settings clean
	+$(MAKE) -C Monitor clean

uninstall:
	sudo rm -rf ~/.stran 
	sudo rm -rf /usr/bin/mstran
	sudo rm -rf /usr/bin/stran
	sudo rm -rf /usr/share/applications/Mstran.desktop
	sudo rm -rf /usr/bin/fetchDict
	sudo rm -rf /usr/bin/extractPic
	sudo rm -rf /usr/bin/stoptran
	sudo rm /usr/lib/libui.so
	sudo rm /usr/lib/libshared.so

prepare:
	bash Bash/prepare.sh
