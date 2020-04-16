CC = gcc

SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj
INCLUDE_DIR = -Iinclude
COMMON_HEADER = -I../CommonHeader/
SHARED_LIB = -L../lib -lshared

CFLAGS= -fno-stack-protector -Wall -lm -lX11 -lXtst -lXext \
		-lpthread -lXfixes -ggdb -g -Wno-deprecated-declarations

LDLIBS = `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0 libbsd libbsd-ctor`

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC)))

MAIN = $(BIN_DIR)/stran

all: $(MAIN)


$(MAIN) : $(OBJS)
	@echo 生成可执行目标文件
	$(CC) $(OBJS) -o $(MAIN) $(CFLAGS) $(LDLIBS) $(SHARED_LIB) 

$(OBJ_DIR)/%.o : ${SRC_DIR}/%.c
	@echo 编译.o文件
	$(CC) -c $(CFLAGS) $(LDLIBS) $(INCLUDE_DIR) $(COMMON_HEADER) -fPIC $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*
