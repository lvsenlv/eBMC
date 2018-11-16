TARGET_BASE_NAME    = test
CC                  = gcc
LD                  = arm-linux-ld
AR                  = arm-linux-ar
OBJCOPY             = arm-linux-objcopy
OBJDUMP             = arm-linux-objdump
CFLAGS              = -Wall -Wno-unused-function -Werror -g
SRC_DIR             = ./src
INC_DIR             = ./inc
OBJ_DIR             = ./obj
LIB_DIR             = ../lib
LIBS                = -L$(LIB_DIR) -lerrno -lfile -lqueue -lpthread
OBJS                = test.o
DEF                 = -D__LINUX 
INC                 = -I./inc -I../inc -I../lib/inc -I../procmonitor/inc
TARGET              = $(TARGET_BASE_NAME)

$(shell mkdir -p $(OBJ_DIR))

all : $(TARGET) 

$(TARGET) : $(OBJS)
	$(CC) $(wildcard $(OBJ_DIR)/*.o) -o $@ $(LIBS)

%.o : $(SRC_DIR)/%.c
	$(CC) -c $<  -o $(OBJ_DIR)/$@ $(DEF) $(INC) $(CFLAGS) 

clean :
	rm -rf $(OBJ_DIR) $(TARGET)
