CC = gcc
CFLAGS=-Wall -lX11 -lXtst -lXext -lpthread -ggdb #-fsanitize=address 
LDLIBS = `pkg-config --cflags --libs gtk+-3.0`

OBJECTS = forDetectMouse.o getClipboard.o GuiEntry.o \
		  notify.o detectMouse.o shmCreate.o newWindow.o\
		  adjustStr.o Main.o


stran : $(OBJECTS)
	$(CC) $(OBJECTS) $(LDLIBS) $(CFLAGS) -o $@
	$(CC) sendToClipboard.c -o sendToClipboard -lX11

#$@为目标集合，@<为依赖集合
$(OBJECTS) : %.o : %.c
	sudo sh prepare.sh
	$(CC) -c $(LDLIBS) $(CFLAGS) $< -o $@  

install:
	sudo cp stran /usr/bin/stran
	sudo cp sendToClipboard /usr/bin/sendToClipboard

clean:
	rm -f *.o stran *.out core.* sendToClipboard
