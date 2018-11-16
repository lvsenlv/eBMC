TARGET_BASE_NAME    = log
CC                  = gcc
LD                  = ld
AR                  = ar
OBJCOPY             = objcopy
OBJDUMP             = objdump
CFLAGS              = -Wall -Wno-unused-function -Werror -O0 -g
SRC_DIR             = ../src
INC_DIR             = ./inc
OBJ_DIR             = ./obj
LIB_DIR             = ./lib
LIBS                = -L$(LIB_DIR) -lpthread
OBJS                = errno_rw.o file.o parser.o hash.o log.o
DEF                 = -D__LINUX
INC                 = -I../inc -I../../inc
TARGET              = $(TARGET_BASE_NAME)

$(shell mkdir -p $(OBJ_DIR))

all : $(TARGET) 

$(TARGET) : $(OBJS)
	$(CC) $(wildcard $(OBJ_DIR)/*.o) -o $@ $(LIBS)

%.o : $(SRC_DIR)/%.c
	$(CC) -c $<  -o $(OBJ_DIR)/$@ $(DEF) $(INC) $(CFLAGS) 

clean :
	rm -rf $(OBJ_DIR) $(TARGET)

