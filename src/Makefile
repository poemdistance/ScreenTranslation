CC = gcc
specific ?= 
SETTINGS_DIR = SettingsInterface

CFLAGS= -fno-stack-protector -Wall -lm -lX11 -lXtst -lXext \
		-lpthread -lXfixes -ggdb -g -Wno-deprecated-declarations

LDLIBS = `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0 libbsd libbsd-ctor`


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

qinstall:
	cp -v stop.sh ~/.stran/
	cp -v startup.sh ~/.stran/
	bash ./stop.sh
	sudo cp -v stop.sh /usr/bin/stoptran
	sudo cp -v stran /usr/bin/
	sudo cp -v mstran /usr/bin/
	sudo cp -v fetchDict /usr/bin/
	sudo cp -v Mstran.desktop /usr/share/applications/
	sudo cp -v extractPic /usr/bin
	sudo desktop-file-install /usr/share/applications/Mstran.desktop
	sudo update-desktop-database

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

prepare:
	bash prepare.sh
