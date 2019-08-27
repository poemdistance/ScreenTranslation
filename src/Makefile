CC = gcc
CFLAGS=-Wall -lX11 -lXtst -lXext -lpthread -ggdb #-fsanitize=address 
LDLIBS = `pkg-config --cflags --libs gtk+-3.0`

OBJECTS = forDetectMouse.o getClipboard.o GuiEntrance.o \
		  notify.o detectMouse.o shmCreate.o newWindow.o\
		  separateData.o cleanup.o adjustStr.o Main.o\
		  countLines.o countCharNums.o adjustStrForScrolledWin.o\

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
	$(CC) -c  $< -o $@  

install:
	sudo cp -v stran /usr/bin/stran
	sudo cp -v mstran /usr/bin/mstran

clean:
	rm -f *.o stran *.out core.* sendToClipboard mstran StranMonitor
