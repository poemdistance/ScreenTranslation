#gcc `pkg-config --cflags --libs gtk+-3.0` forDetectMouse.c getClipboard.c All.c -lX11 -lXtst -lXext -lpthread -ggdb

CC = gcc
CFLAGS=-Wall -lX11 -lXtst -lXext -lpthread -ggdb
LDLIBS = `pkg-config --cflags --libs gtk+-3.0`

OBJECTS = forDetectMouse.o getClipboard.o GuiEntry.o \
			detectMouse.o shmCreate.o newWindow.o adjustStr.o All.o

main : $(OBJECTS)
	$(CC) $(OBJECTS) $(LDLIBS) $(CFLAGS) -o $@

#$@为目标集合，@<为依赖集合
$(OBJECTS) : %.o : %.c
	$(CC) -c $(LDLIBS) $(CFLAGS) $< -o $@  

clean:
	rm -f *.o main *.out core.*
