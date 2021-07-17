CC = gcc

SRC_DIR = ../src
LIB_DIR = ../../lib
OBJ_DIR = ../obj
INCLUDE_DIR = -I../include 
COMMON_HEADER = -I../../CommonHeader/

CFLAGS= -fno-stack-protector -Wall -lm -lX11 -lXtst -lXext \
		-lpthread -lXfixes -Wno-deprecated-declarations

LDLIBS = `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0 libbsd libbsd-ctor`

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC)))

SHARED_LIB = $(LIB_DIR)/libui.so 
all: $(SHARED_LIB)

$(SHARED_LIB) : $(OBJS)
	$(CC) $(CFLAGS) $(LDLIBS) $(OBJS) -L$(LIB_DIR) -lshared -shared -o $(SHARED_LIB)

$(OBJ_DIR)/%.o : ${SRC_DIR}/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(LDLIBS) $(INCLUDE_DIR) $(COMMON_HEADER) -fPIC -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(LIB_DIR)/libui.so
