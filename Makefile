#
# This is a Top Makefile.
#

srctree := $(shell pwd)

BUILD_OUTPUT := $(srctree)/out

ifeq ("$(origin O)", "command line")
  BUILD_OUTPUT := $(O)
endif

BUILD_OUTPUT := $(shell mkdir -p $(BUILD_OUTPUT) && cd $(BUILD_OUTPUT) && /bin/pwd)

export srctree BUILD_OUTPUT

include $(srctree)/tools/scripts/project.mk



