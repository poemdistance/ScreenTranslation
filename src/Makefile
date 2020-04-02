CC = gcc

specific ?= 

CFLAGS= -fno-stack-protector -Wall -lm -lX11 -lXtst -lXext -lpthread -lXfixes -ggdb -g -Wno-deprecated-declarations#-fsanitize=address 
LDLIBS = `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0 libbsd libbsd-ctor`

OBJECTS = forDetectMouse.o getClipboard.o GuiEntrance.o \
		  notify.o detectMouse.o newNormalWin.o\
		  separateData.o cleanup.o adjustStr.o Main.o \
		  countLines.o countCharNums.o adjustStrForScrolledWin.o\
		  audioPlayer.o background.o switchButton.o checkSelectionChanged.o\
		  keyPressEvent.o quickSearch.o findKeyboardDevice.o\
		  shortcutListener.o  searchWindow.o tranSelect.o isEmpty.o \
		  tranPicActionDetect.o memoryControl.o\
		  expanduser.o dataStatistics.o getPointerPosition.o strmask.o\
		  useful.o configControl.o shmCreate.o


#公用文件
COMMON = useful.o configControl.o expanduser.o strmask.o shmCreate.o

MONNITOR = StranMonitor.o gtkLabel.o iconShowTimeSetting.o removeWidget.o \
		   shortcutSetting.o windowPositionSetting.o focus.o setting.o \
		   settingWindow.o tray.o iconPositionSetting.o settingList.o \
		   windowPrefSetting.o


all : stran mstran

stran : $(OBJECTS)
	$(CC) $(OBJECTS) $(LDLIBS) $(CFLAGS) -o $@

mstran : $(MONNITOR) $(COMMON)
	$(CC) $(MONNITOR) $(COMMON) $(LDLIBS) $(CFLAGS) -o $@


#$@为目标集合，@<为依赖集合
$(OBJECTS) : %.o : %.c
	$(CC) -c $(LDLIBS) $(CFLAGS) $< -o $@  

$(MONNITOR) : %.o : %.c
	# $(CC) $(LDLIBS) $(CFLAGS) -c  $< -o $@  -Wall
	$(CC) $(LDLIBS) $(CFLAGS) -c  $< -o $@  -Wall

install:
	# bash ./stop.sh
	sudo cp -v stran /usr/bin/
	sudo cp -v stop.sh /usr/bin/stoptran
	sudo cp -v mstran /usr/bin/
	sudo cp -v fetchDict /usr/bin/
	sudo cp -v Mstran.desktop /usr/share/applications/
	sudo cp -v extractPic /usr/bin
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
	rm -f *.o stran *.out core.* sendToClipboard mstran StranMonitor checkSelectionChanged

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
