CC = gcc
CFLAGS=-Wall -lX11 -lXtst -lXext -lpthread -ggdb
LDLIBS = `pkg-config --cflags --libs gtk+-3.0`

OBJECTS = forDetectMouse.o getClipboard.o GuiEntry.o \
			notify.o detectMouse.o shmCreate.o newWindow.o adjustStr.o Main.o

stran : $(OBJECTS)
	$(CC) $(OBJECTS) $(LDLIBS) $(CFLAGS) -o $@

#$@为目标集合，@<为依赖集合
$(OBJECTS) : %.o : %.c
	$(CC) -c $(LDLIBS) $(CFLAGS) $< -o $@  

clean:
	rm -f *.o stran *.out core.*
