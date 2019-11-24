CC = gcc
CFLAGS=-Wall -lX11 -lXtst -lXext -lpthread -lXfixes -ggdb -g #-fsanitize=address 
LDLIBS = `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0`

OBJECTS = forDetectMouse.o getClipboard.o GuiEntrance.o \
		  notify.o detectMouse.o shmCreate.o newWindow.o\
		  separateData.o cleanup.o adjustStr.o Main.o\
		  countLines.o countCharNums.o adjustStrForScrolledWin.o\
		  audioPlayer.o background.o switchButton.o checkSelectionChanged.o\
		  keyPressEvent.o quickSearch.o findKeyboardDevice.o\
		  captureShortcutEvent.o  searchWindow.o tranSelect.o isEmpty.o \
		  tranPicActionDetect.o

MONNITOR = StranMonitor.o

all : stran mstran

stran : $(OBJECTS)
	$(CC) $(OBJECTS) $(LDLIBS) $(CFLAGS) -o $@

mstran : $(MONNITOR)
	$(CC) StranMonitor.c -o mstran


#$@为目标集合，@<为依赖集合
$(OBJECTS) : %.o : %.c
	$(CC) -c $(LDLIBS) $(CFLAGS) $< -o $@  

$(MONNITOR) : %.o : %.c
	$(CC) -c  $< -o $@  -Wall

install:
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

prepare:
	bash prepare.sh
