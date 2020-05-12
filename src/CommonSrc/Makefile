CC = gcc

SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj
INCLUDE_DIR = -Iinclude
COMMON_LIB_DIR = ../lib
COMMON_HEADER = -I../CommonHeader/

CFLAGS= -fno-stack-protector -Wall -lm -lX11 -lXtst -lXext \
		-lpthread -lXfixes -ggdb -g -Wno-deprecated-declarations

LDLIBS = `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0 libbsd libbsd-ctor`

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC)))

SHARED_LIB = $(COMMON_LIB_DIR)/libshared.so

all: $(SHARED_LIB)


$(SHARED_LIB) : $(OBJS)
	$(CC) $(INCLUDE_DIR) $(COMMON_HEADER) $(OBJS) -shared -o $(SHARED_LIB)

$(OBJ_DIR)/%.o : ${SRC_DIR}/%.c
	@mkdir -p $(COMMON_LIB_DIR)
	@mkdir -p $(OBJ_DIR)
	@echo 编译.o文件
	$(CC) -c $(CFLAGS) $(LDLIBS) $(INCLUDE_DIR) $(COMMON_HEADER) -fPIC $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(COMMON_LIB_DIR)/*
	rm -rf $(COMMON_LIB_DIR)/libshared.so
