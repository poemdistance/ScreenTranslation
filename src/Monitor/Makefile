CC = gcc

SRC_DIR = src
BIN_DIR = bin
LIB_DIR = lib
OBJ_DIR = obj
INCLUDE_DIR = -I../CommonHeader
LIB = -L../lib/ -lui -lshared

CFLAGS= -fno-stack-protector -Wall -lm -lX11 -lXtst -lXext \
		-lpthread -lXfixes -ggdb -g -Wno-deprecated-declarations

LDLIBS = `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0 libbsd libbsd-ctor`

#公用文件
COMMON_SRC = useful.c configControl.c expanduser.c strmask.c shmCreate.c
COMMON_OBJ = $(join $(addsuffix obj/, $(dir $(COMMON_SRC))), $(notdir $(COMMON_SRC:.c=.o)))

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC)))

MAIN = $(BIN_DIR)/mstran

all: $(MAIN)


$(MAIN) : $(OBJS)
	@echo 生成可执行目标文件
	$(CC) $(OBJS) -o $(MAIN) $(LIB) $(CFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o : ${SRC_DIR}/%.c
	@echo 编译.o文件
	$(CC) -c $(CFLAGS) $(LDLIBS) $(INCLUDE_DIR) $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(LIB_DIR)/*
