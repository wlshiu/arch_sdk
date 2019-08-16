#
# Component Makefile
#

COMPONENT_ADD_LDFLAGS = -l$(COMPONENT_NAME)
COMPONENT_ADD_INCLUDEDIRS := include
COMPONENT_PRIV_INCLUDEDIRS := include portable/GCC/ARM_CM3

COMPONENT_OBJS := \
	portable/GCC/ARM_CM3/port.o \
	portable/MemMang/heap_4.o \
	croutine.o \
	event_groups.o \
	list.o \
	queue.o \
	stream_buffer.o \
	tasks.o \
	timers.o

COMPONENT_SRCDIRS := . \
	portable/GCC/ARM_CM3 \
	portable/MemMang

