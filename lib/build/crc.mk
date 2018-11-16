TARGET_BASE_NAME    = crc
CC                  = gcc
LD                  = ld
AR                  = ar
OBJCOPY             = objcopy
OBJDUMP             = objdump
CFLAGS              = -Wall -Wno-unused-function -Werror -O3
SRC_DIR             = ../src
INC_DIR             = ./inc
OBJ_DIR             = .
LIB_DIR             = ./lib
LIBS                = -L$(LIB_DIR) -lpthread
OBJS                = crc.o
DEF                 = -D__LINUX
INC                 = -I../inc -I$(COMMON)
TARGET              = $(TARGET_BASE_NAME)

#$(shell mkdir -p $(OBJ_DIR))

all : $(TARGET) 

$(TARGET) : $(OBJS)
	@#$(AR) rcs ../lib$@.a $(wildcard $(OBJ_DIR)/*.o)
	$(AR) rcs ../lib$@.a $(OBJS)

%.o : $(SRC_DIR)/%.c
	$(CC) -c $<  -o $(OBJ_DIR)/$@ $(DEF) $(INC) $(CFLAGS) 

clean :
	rm -f $(OBJS) ../lib$(TARGET).a
