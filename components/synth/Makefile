CC = g++
TARGET ?= test
SRC_DIRS ?= ./src
 
SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)
INCLUDE := -I./include
LDFLAGS= -g -Wall -lm -lgcc
 
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS)
 
.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)
 
-include $(DEPS)
