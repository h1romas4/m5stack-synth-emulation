#
# Main Makefile. This is basically the same as a component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_SRCDIRS := src
COMPONENT_ADD_INCLUDEDIRS := src

CFLAGS += -Wno-unused-result
CFLAGS += -Wno-unused-function
CFLAGS += -DESP32_GENSOUND=1
CFLAGS += -DBUILD_YM2612
