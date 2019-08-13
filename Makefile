#
# This is a Top Makefile.
#

srctree := $(shell pwd)

BUILD_PATH := $(srctree)/out

ifeq ("$(origin O)", "command line")
  BUILD_PATH := $(O)
endif

BUILD_PATH := $(shell mkdir -p $(BUILD_PATH) && cd $(BUILD_PATH) && /bin/pwd)

export srctree BUILD_PATH

include $(srctree)/tools/scripts/project.mk



