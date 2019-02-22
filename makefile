# Makefile Written by ticktick

# Show how to cross-compile c/c++ code for android platform

.PHONY: clean

CC = gcc
AR = ar
LD = ld
STRIP = strip
WALL = -g -Wall

INCLUDE = -I/usr/include -I/usr/local/include -I/usr/local/include/event2
STATIC_LIB = /usr/local/lib/libevent.a /usr/local/lib/libevent_pthreads.a
#-lssl -lcrypto

TARGET_NAME = trace_server

CUR_DIR = $(shell pwd)
TARGET = $(CUR_DIR)/$(TARGET_NAME)

SRC_DIR = $(CUR_DIR)/src
OBJ_DIR = $(CUR_DIR)/obj

#获取当前目录下所有的.c文件
SOURCE = $(wildcard $(SRC_DIR)/*.c)

#得到相应的.o文件,所有的.c文件替换为.o文件
OBJ = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SOURCE)))

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	$(CC) $(WALL) $(INCLUDE)  $< -c -o $@

#生成目标的依赖
$(TARGET): $(OBJ) $(OBJ_CPP)
	@echo "--------------create $(TARGET)---------"
	$(CC) $(INCLUDE) -o $@ $^ $(STATIC_LIB) -lpthread -lrt -ldl
	$(STRIP) $(TARGET)
	du -h $(TARGET)
clean:
	rm -rf $(OBJ_DIR)/*.o
	rm $(TARGET)