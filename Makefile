#-------------------------------------------------------------------------------
# Makefile
#-------------------------------------------------------------------------------
# SRC
SRC_DIR = .

SRCS  = $(SRC_DIR)/image.cc \
        $(SRC_DIR)/main.cc

OBJS  = $(SRCS:.cc=.o)
TARGET = test

#
# makefile config
#
CC = g++ -std=c++11
DEBUGFLAG = -g
OPTFLAG = -O3 -march=native
INC  = -I$(SRC_DIR)

CFLAGS  = -c $(DEBUGFLAG) $(OPTFLAG) $(DEFS) -fPIC
LIBS    =

LFLAGS = -lz -lc -lm

.SUFFIXES: .cc .o

default: all

all: $(TARGET)

.cc.o:
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $@  $(LIBS) $(LFLAGS)  $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET) tags .gdbinit images/*
