CC = gcc
TARGET ?= vgmplay
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)
INCLUDE := -I./include
LDFLAGS= -g -lm -lgcc

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS) $(LDFLAGS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)
