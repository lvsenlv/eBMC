TARGET_BASE_NAME    = procmonitor
CC                  = gcc
LIBS                = -static
CFLAGS              = -Wall -Wno-unused-function -Werror
SRC_DIR             = ./src
OBJ_DIR             = ./obj
LIB_DIR             = ./lib
OBJS                = procmonitor.o
DEF                 = 
INC                 = -I./inc -I../inc
TARGET              = $(TARGET_BASE_NAME)

$(shell mkdir -p $(OBJ_DIR))

all : $(TARGET) 

$(TARGET) : $(OBJS)
	$(CC) $(wildcard $(OBJ_DIR)/*.o) -o $@ $(LIBS)

%.o : $(SRC_DIR)/%.c
	$(CC) -c $<  -o $(OBJ_DIR)/$@ $(DEF) $(INC) $(CFLAGS) 

clean :
	rm -rf $(OBJ_DIR) $(TARGET)
