#
# Main Makefile. This is basically the same as a component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_SRCDIRS := src
COMPONENT_OBJS := src/sn76489.o src/panning.o src/ym2612.o
COMPONENT_ADD_INCLUDEDIRS := src

CFLAGS := -Wno-unused-result
CFLAGS := -Wno-unused-function
CFLAGS := -mlongcalls
CPPFLAGS := -DESP32_SYNTH
